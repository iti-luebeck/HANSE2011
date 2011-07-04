
#include <QtCore>
#include "module_cutter.h"
#include "form_cutter.h"

Module_Cutter::Module_Cutter(QString moduleId)
    :RobotModule(moduleId)
{

}

Module_Cutter::~Module_Cutter()
{

}

void Module_Cutter::init()
{
    reset();
}

void Module_Cutter::reset()
{
    RobotModule::reset();
}

QList<RobotModule*> Module_Cutter::getDependencies()
{
    return QList<RobotModule*>();
}

QWidget* Module_Cutter::createView(QWidget* parent)
{
    return new FormUID(this, parent);
}

void Module_Cutter::terminate()
{
    RobotModule::terminate();
}

