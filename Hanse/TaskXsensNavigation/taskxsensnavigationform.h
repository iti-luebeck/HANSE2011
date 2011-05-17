#ifndef TASKXSENSNAVIGATIONFORM_H
#define TASKXSENSNAVIGATIONFORM_H

#include <TaskXsensNavigation/taskxsensnavigation.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskXsensFollowing;
class Module_Simulation;

namespace Ui {
    class TaskXsensNavigationForm;
}

class TaskXsensNavigationForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskXsensNavigationForm(TaskXsensNavigation *txn, QWidget *parent = 0);
    ~TaskXsensNavigationForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskXsensNavigationForm *ui;

    Log4Qt::Logger *logger;
    TaskXsensNavigation* taskxsensnavigation;

signals:
    void newSchDesSignal(QString taskName, QString newD);
    void updateTaskSettingsSignal();

public slots:
    void on_applyButton_clicked();
};

#endif // TASKXSENSNAVIGATIONFORM_H
