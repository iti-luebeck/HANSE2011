#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "modulesgraph.h"
#include "healthmodel.h"
#include "datamodel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    ModulesGraph graph;
    HealthModel *healthModel;
    DataModel *dataModel;
    QSettings settings;
    void readSettings();
    void writeSettings();
    void setupLog4Qt();

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

private slots:
    void disableAll();
    void enableAll();
};

#endif // MAINWINDOW_H