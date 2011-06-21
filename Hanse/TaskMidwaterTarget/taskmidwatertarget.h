#ifndef TASKMIDWATERTARGET_H
#define TASKMIDWATERTARGET_H

#include <Framework/robotbehaviour.h>

class Module_Simulation;
class Module_Navigation;


#define TASK_STATE_START                    "task started"
#define TASK_STATE_END                      "task finished"
#define TASK_STATE_END_FAILED               "task finished - unsuccessful"

class TaskMidwaterTarget : public RobotBehaviour
{
    Q_OBJECT
public:
    TaskMidwaterTarget(QString id, Module_Simulation *sim, Module_Navigation *n);

    QWidget* createView(QWidget *parent);

    QList<RobotModule*> getDependencies();

    bool isActive();

    QString getTaskState();

    QTimer taskTimer;

private:
    Module_Simulation *sim;
    Module_Navigation *navi;

    void init();
    bool running;
    void terminate();
    void reset();

    QString taskState;

signals:
    void updateSettings();
    void newState(QString state);
    void newStateOverview(QString state);

public slots:
    void startBehaviour();
    void stop();
    void emergencyStop();
    void timeoutStop();
    void controlEnabledChanged(bool b);

};

#endif // TASKMIDWATERTARGET_H
