#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include <framework.h>
#include <module_scanningsonar.h>
//#include <qextserialenumerator.h>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    logger = Log4Qt::Logger::logger("MainWindow");

    ui->setupUi(this);

    setupLog4Qt();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse");

    settings.setValue("gui2/pi", 3.14);
    settings.value("gui2/pi").toFloat();

    QList<RobotModule*> list = graph.getModules();

    for (int i = 0; i < list.size(); ++i) {
        RobotModule* m = list.at(i);
        QWidget* w = m->createView(ui->tabWidget);
        ui->tabWidget->addTab(w, m->getTabName());

        // TODO: free action later
        QAction *action = new QAction(m->getTabName(), ui->menuReset);
        ui->menuReset->addAction(action);
        connect(action, SIGNAL(triggered()), m, SLOT(reset()));
        connect(ui->actionAll_Modules, SIGNAL(triggered()), m, SLOT(reset()));

        QAction *action2 = new QAction(m->getTabName(), ui->menuEnabled);
        action2->setCheckable(true);
        ui->menuEnabled->addAction(action2);
        connect(action2, SIGNAL(triggered(bool)), m, SLOT(enabled(bool)));
        connect(m, SIGNAL(isEnabled(bool)), action2, SLOT(setChecked(bool)));

        // TODO: enable/disable all
    }

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
    logger->info("Have a nice day!");
}

void MainWindow::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse");
    settings.setValue("gui/pos", pos());
    settings.setValue("gui/size", size());
}

void MainWindow::readSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse");
    QPoint pos = settings.value("gui/pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("gui/size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::setupLog4Qt()
{
    QSettings s;

    // Set logging level for Log4Qt to TRACE
    s.beginGroup("Log4Qt");
    s.setValue("Debug", "TRACE");

    // Configure logging to log to the file using the level TRACE
    s.beginGroup("Properties");
    s.setValue("log4j.appender.A1", "org.apache.log4j.FileAppender");
    s.setValue("log4j.appender.A1.file", "hanseGUI.log");
    s.setValue("log4j.appender.A1.layout", "org.apache.log4j.TTCCLayout");
    s.setValue("log4j.appender.A1.layout.DateFormat", "ISO8601");
    s.setValue("log4j.rootLogger", "TRACE, A1");

    // TODO: another appender inside the framework
    // Settings will become active on next application startup
}
