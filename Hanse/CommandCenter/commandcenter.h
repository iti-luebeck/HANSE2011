#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>
#include <Framework/eventthread.h>
#include <TestTask/testtask.h>
#include <TestTask2/testtask2.h>

class Module_Simulation;
class Module_ThrusterControlLoop;
class Module_HandControl;
class Module_PressureSensor;
class Behaviour_PipeFollowing;
class Behaviour_BallFollowing;
class Behaviour_TurnOneEighty;
class Behaviour_WallFollowing;

class CommandCenter : public RobotModule
{
    Q_OBJECT
public:
    CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TestTask *tt, TestTask2 *tt2);

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    bool isActive();

   //void pleaseStop();
   //void run(void);
    QList<QString> schedule;

    QString lTask;

    QTimer controlTimer;

private:
   CommandCenter* c;
   Module_Simulation* sim;
   Module_ThrusterControlLoop* tcl;
   Module_HandControl* handControl;
   Module_PressureSensor* pressure;

   bool running;

    EventThread updateThread;
    TestTask *testtask;
    TestTask2 *testtask2;
   void commandCenterControl();
    int count;

   QTimer depthWaitTimer;

   void submergedExecute();

public slots:
    void reset();
    void terminate();

   // void startCommandCenter(QList<QString> s);
    void startCC();
    void stopCC();

    void finishedControl(RobotBehaviour*, bool success);
    void timeout();

private slots:
    //void gotEnabledChanged(bool);
    void doNextTask();

signals:
    void error();
    void currentTask(QString s);
    void newError(QString s);
    void newAborted(QString s);


    void setDepth(float depth);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
    void stopAllTasks();
    void resetTCL();

    void startTestTask();
    void stopTestTask();

    void startTestTask2();
    void stopTestTask2();

    void taskTimeout();

    void newList(QString s);

    void cStop();

private:
    QTimer timer;

    void init();
};

#endif // COMMANDCENTER_H
