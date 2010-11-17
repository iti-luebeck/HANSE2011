#ifndef ROBOTMODULE_MT_H
#define ROBOTMODULE_MT_H

#include <Framework/robotmodule.h>

class DataRecorder;

class RobotModule_MT : public RobotModule
{
    Q_OBJECT

public:

    /**
      * Create a new Robot Module with the given id.
      */
    RobotModule_MT(QString id);

    virtual bool waitForThreadToStop(unsigned long timeout);
    virtual bool waitForThreadToStop();

public slots:
    virtual void terminate();
protected:

    /**
      * sleep for the given amount of milliseconds
      */
    virtual void msleep(int millies);


private:

    class MyModuleThread: public QThread
    {
    public:
        static void msleep(int millies);
        void run();
        void printID(QString id);
//        int getID();
    };


    MyModuleThread moduleThread;




};

#endif // RobotModule_MT_H
