#include "Framework/robotmodule_mt.h"
#include "datarecorder.h"

RobotModule_MT::RobotModule_MT(QString id)
    : RobotModule(id)
{
    logger->debug(id);
    moduleThread.start(QThread::NormalPriority);
    this->moveToThread(&moduleThread);
}

bool RobotModule_MT::waitForThreadToStop(unsigned long timeout)
{
   return this->moduleThread.wait(timeout);
}

bool RobotModule_MT::waitForThreadToStop()
{
    return this->moduleThread.wait();
}

void RobotModule_MT::msleep(int millies)
{
    MyModuleThread::msleep(millies);
}

void RobotModule_MT::terminate()
{
    RobotModule::terminate();
    moduleThread.exit(0);
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

void RobotModule_MT::MyModuleThread::printID(QString id)
{
    qDebug() << id;
    qDebug() << "ausm THREAD ID";
    qDebug() << QThread::currentThreadId();
}

