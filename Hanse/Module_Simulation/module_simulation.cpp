#include "module_simulation.h"
#include "simulation_form.h"

Module_Simulation::Module_Simulation(QString id)
    : RobotModule(id)
{
    thread.start();

    //this->uid=uid;

    setDefaultValue("i2cAddress", 0x50);
    setDefaultValue("frequency", 1);

    thread.start();
    //timer.moveToThread(&thread);

    reset();
}

Module_Simulation::~Module_Simulation()
{
}

void Module_Simulation::terminate()
{
    RobotModule::terminate();
    QTimer::singleShot(0, &timer, SLOT(stop()));
}

void Module_Simulation::reset()
{
    RobotModule::reset();
}

QList<RobotModule*> Module_Simulation::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_Simulation::createView(QWidget* parent)
{
    return new Simulation_Form(this, parent);
}

void Module_Simulation::doHealthCheck()
{
    if (!isEnabled())
        return;

    setHealthToOk();
}
