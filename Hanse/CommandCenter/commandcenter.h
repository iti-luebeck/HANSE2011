#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include <Framework/robotmodule.h>
#include <QtCore>
#include <Framework/eventthread.h>
#include <TestTask/testtask.h>
#include <TestTask2/testtask2.h>

class Module_Simulation;

class CommandCenter : public RobotModule
{
    Q_OBJECT
public:
    CommandCenter(QString id, Module_Simulation *sim, TestTask *tt, TestTask2 *tt2);

    QWidget* createView(QWidget *parent);
    QList<RobotModule*> getDependencies();

    bool isActive();

   //void pleaseStop();
   //void run(void);
    QList<QString> schedule;
private:
   CommandCenter* c;
   Module_Simulation* sim;


   bool running;

    EventThread updateThread;
    TestTask *testtask;
    TestTask2 *testtask2;
   void commandCenterControl();
public slots:
    void reset();
    void terminate();

   // void startCommandCenter(QList<QString> s);
    void startCC();
    void stopCC();

    void finishedControl(RobotBehaviour*, bool success);

private slots:
    //void gotEnabledChanged(bool);

signals:
    void error();
    void nData(QString s);

private:
    QTimer timer;

    void init();
};

#endif // COMMANDCENTER_H
