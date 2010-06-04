#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse"),
    ui(new Ui::MainWindow)
{

    logger = Log4Qt::Logger::logger("MainWindow");

    graph.build();

    ui->setupUi(this);

    healthModel = new HealthModel(&graph);
    dataModel = new DataModel(&graph);

    ui->healthView->setModel(healthModel);
    ui->dataView->setModel(dataModel);

    QList<RobotModule*> list = graph.getModules();

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

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    settings.setValue("gui/pos", pos());
    settings.setValue("gui/size", size());
}

void MainWindow::readSettings()
{
    QPoint pos = settings.value("gui/pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("gui/size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
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

        m->setEnabled(true);
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
