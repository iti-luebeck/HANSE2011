#include "Framework/robotmodule_mt.h"
#include "datarecorder.h"

RobotModule_MT::RobotModule_MT(QString id)
    : RobotModule(id)
{
    moduleThread.start(QThread::NormalPriority);
    this->moveToThread(&moduleThread);
}

void RobotModule_MT::msleep(int millies)
{
    MyModuleThread::msleep(millies);
}

void RobotModule_MT::MyModuleThread::msleep(int millies)
{
    QThread::msleep(millies);
}

void RobotModule_MT::MyModuleThread::run()
{
    qDebug() << "ausm THREAD ID";
    qDebug() << QThread::currentThreadId();
    QThread::exec();

}

