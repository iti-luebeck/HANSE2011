#ifndef TASKWALLNAVIGATIONFORM_H
#define TASKWALLNAVIGATIONFORM_H

#include <TaskWallNavigation/taskwallnavigation.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskWallFollowing;
class Module_Simulation;

namespace Ui {
    class TaskWallNavigationForm;
}

class TaskWallNavigationForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskWallNavigationForm(TaskWallNavigation *twn, QWidget *parent = 0);
    ~TaskWallNavigationForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskWallNavigationForm *ui;

    Log4Qt::Logger *logger;
    TaskWallNavigation* taskwallnavigation;

signals:
    void newSchDesSignal(QString taskName, QString newD);
    void updateTaskSettingsSignal();

public slots:
    void on_applyButton_clicked();
    void returnDescription();

};

#endif // TASKWALLNAVIGATIONFORM_H
