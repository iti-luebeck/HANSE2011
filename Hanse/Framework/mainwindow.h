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
    QList<QWidget*> openTabs;
    QStringList openTabIds;
    void readSettings();
    void writeSettings();
    void setupLog4Qt();
    QWidget* openNewTab(RobotModule* m);

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

private slots:
    void on_healthView_doubleClicked(QModelIndex index);
    void on_tabWidget_tabCloseRequested(int index);
    void on_filter_textChanged(QString );
    void disableAll();
    void enableAll();
};

#endif // MAINWINDOW_H
