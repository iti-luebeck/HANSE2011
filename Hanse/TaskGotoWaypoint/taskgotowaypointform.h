#ifndef TASKGOTOWAYPOINTFORM_H
#define TASKGOTOWAYPOINTFORM_H

#include <TaskGotoWaypoint/taskgotowaypoint.h>

#include <QWidget>
#include <log4qt/logger.h>

class TaskGotoWaypoint;
class Module_Simulation;

namespace Ui {
    class TaskGotoWaypointForm;
}

class TaskGotoWaypointForm : public QWidget
{
    Q_OBJECT

public:
    explicit TaskGotoWaypointForm(TaskGotoWaypoint *tgw, QWidget *parent = 0);
    ~TaskGotoWaypointForm();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TaskGotoWaypointForm *ui;

    Log4Qt::Logger *logger;
    TaskGotoWaypoint* taskgotowaypoint;

signals:
    void updateTaskSettingsSignal();

public slots:
    void on_applyButton_clicked();

};

#endif // TASKGOTOWAYPOINTFORM_H
