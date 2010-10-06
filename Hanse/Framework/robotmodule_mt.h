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


//signals:


//public slots:



protected:



    /**
      * sleep for the given amount of milliseconds
      */
    void msleep(int millies);

//protected slots:


private:
    class MyQThread: public QThread
    {
    public:
        static void msleep(int millies);
        void run();
    };




};

#endif // RobotModule_MT_H
