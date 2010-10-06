#include "Framework/robotmodule_mt.h"
#include "datarecorder.h"

RobotModule_MT::RobotModule_MT(QString id)
    : RobotModule(id)
{

}


void RobotModule_MT::msleep(int millies)
{
    MyQThread::msleep(millies);
}

void RobotModule_MT::MyQThread::msleep(int millies)
{
    QThread::msleep(millies);
}

void RobotModule_MT::MyQThread::run()
{
    exec();

}
