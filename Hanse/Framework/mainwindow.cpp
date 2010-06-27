#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Framework/moduledataview.h>
#include <Framework/modulehealthview.h>
#include <Framework/qclosabledockwidget.h>
#include <Framework/robotbehaviour.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse"),
    ui(new Ui::MainWindow)
{

    logger = Log4Qt::Logger::logger("MainWindow");

    graph.build();

    ui->setupUi(this);

    healthModel = new HealthModel(this, &graph);
    dataModel = new DataModel(this, &graph);

    ui->healthView->setModel(healthModel);
    ui->dataView->setModel(dataModel);

    QList<RobotModule*> list = graph.getModules();

    statusbarLabel = new QLabel(ui->statusBar);
    ui->statusBar->addWidget(statusbarLabel);

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
    timer.start(500);

    QStringList oldOpenTabs = settings.value("openTabs").toStringList();
    for (int i = 0; i < list.size(); ++i) {
        RobotModule* m = list.at(i);

        // create widget
        if (oldOpenTabs.contains(m->getId()))
            openNewTab(m);

        // TODO: free actions later

        QAction *actionReset = new QAction(m->getTabName(), ui->menuReset);
        ui->menuReset->addAction(actionReset);
        connect(actionReset, SIGNAL(triggered()), m, SLOT(reset()));
        connect(ui->actionAll_Modules, SIGNAL(triggered()), m, SLOT(reset()));

        QAction *actionEnabled = new QAction(m->getTabName(), ui->menuEnabled);
        actionEnabled->setCheckable(true);
        ui->menuEnabled->addAction(actionEnabled);
        actionEnabled->setChecked(m->isEnabled());
        connect(actionEnabled, SIGNAL(triggered(bool)), m, SLOT(setEnabled(bool)));
        connect(m, SIGNAL(enabled(bool)), actionEnabled, SLOT(setChecked(bool)));

    }

    connect(ui->actionDisable_All, SIGNAL(triggered()), this, SLOT(disableAll()));
    connect(ui->actionEnable_All, SIGNAL(triggered()), this, SLOT(enableAll()));
    connect(ui->actionNew_window, SIGNAL(triggered()), this, SLOT(openNewDataWindow()));
    connect(ui->actionHealthDockItem, SIGNAL(triggered()), this, SLOT(openNewHealthWindow()));

    QSettings sets;
    sets.beginGroup("docks");

    QStringList s = sets.value("openHealthWidgets").toStringList();
    s.removeDuplicates();
    foreach (QString uuid, s) {
        QClosableDockWidget *dockWidget = new QClosableDockWidget("Health", this, uuid);
        dockWidget->setObjectName(uuid);
        dockWidget->setWidget(new ModuleHealthView(&graph, dockWidget));
        addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    }

    QStringList ds = sets.value("openDataWidgets").toStringList();
    ds.removeDuplicates();
    foreach (QString uuid, ds) {
        QClosableDockWidget *dockWidget = new QClosableDockWidget("Data", this, uuid);
        dockWidget->setObjectName(uuid);
        dockWidget->setWidget(new ModuleDataView(&graph, dockWidget));
        addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
    }

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openNewHealthWindow()
{
     QClosableDockWidget *dockWidget = new QClosableDockWidget("Health", this, "");
     dockWidget->setWidget(new ModuleHealthView(&graph, dockWidget));
     addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
}

void MainWindow::openNewDataWindow()
{
     QClosableDockWidget *dockWidget = new QClosableDockWidget("Data", this, "");
     dockWidget->setWidget(new ModuleDataView(&graph, dockWidget));
     addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    logger->info("Closing application...");
    graph.HastaLaVista();
    writeSettings();
    event->accept();
    logger->info("Application terminated. Have a nice day!");
}

void MainWindow::writeSettings()
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::disableAll()
{
    QList<RobotModule*> list = graph.getModules();

    for (int i = 0; i < list.size(); ++i) {
        RobotModule* m = list.at(i);

        m->setEnabled(false);
    }
}


void MainWindow::enableAll()
{
    QList<RobotModule*> list = graph.getModules();

    for (int i = 0; i < list.size(); ++i) {
        RobotModule* m = list.at(i);
        RobotBehaviour* behav = dynamic_cast<RobotBehaviour*>(m);
        if (!behav) {
            m->setEnabled(true);
        }
    }
}


void MainWindow::on_filter_textChanged(QString filter)
{
    dataModel->setFilter(filter);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    QWidget* w = ui->tabWidget->widget(index);

    if (w==ui->dataTab || w==ui->healthTab)
        return;

    ui->tabWidget->removeTab(index);
    openTabs.removeAll(w);
    openTabIds.removeAll(w->objectName());
    settings.setValue("openTabs",openTabIds);
    delete w;
}

void MainWindow::on_healthView_doubleClicked(QModelIndex index)
{
    RobotModule* m = graph.getModules().at(index.row());

    QWidget* widget = NULL;
    foreach(QWidget* w, openTabs) {
        if (w->objectName()==m->getId()) {
            widget=w;
        }
    }

    if (!widget) {
        widget = openNewTab(m);
    }

    int i = ui->tabWidget->indexOf(widget);
    ui->tabWidget->setCurrentIndex(i);
}

QWidget* MainWindow::openNewTab(RobotModule* m) {
    QWidget* widget = m->createView(ui->tabWidget);
    widget->setObjectName(m->getId());
    ui->tabWidget->addTab(widget, m->getTabName());
    openTabs.append(widget);
    openTabIds.append(m->getId());
    settings.setValue("openTabs",openTabIds);
    return widget;
}

void MainWindow::updateStatusBar()
{
    int badModules = 0;
    int errorCount = 0;
    foreach (RobotModule* r, graph.getModules()) {
        if (!r->getHealthStatus().isHealthOk())
            badModules++;

        errorCount += r->getHealthStatus().getErrorCount();
    }

    statusbarLabel->setText(QString("Health status: %1/%2 sick modules. Total error count: %3")
                            .arg(badModules).arg(graph.getModules().size()).arg(errorCount));
}
