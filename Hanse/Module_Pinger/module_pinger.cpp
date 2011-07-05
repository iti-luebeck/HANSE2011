#include <Module_Pinger/module_pinger.h>
#include <Module_Pinger/form_pinger.h>


Module_Pinger::Module_Pinger(QString id):RobotModule(id)
{

    setEnabled(false);

}

void Module_Pinger::init()
{
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabled(bool)));
}

QList<RobotModule*> Module_Pinger::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_Pinger::createView(QWidget *parent)
{
    return new FormPinger(this, parent);
}

void Module_Pinger::reset()
{

    RobotModule::reset();
}



void Module_Pinger::terminate()
{

    RobotModule::terminate();
}

void Module_Pinger::gotEnabled(bool value)
{
    Q_UNUSED(value);
//    if(value)
//    {
//        timer.start(this->getSettingsValue("timeout").toInt());
//    }
//    else
//    {
//        timer.stop();
//    }

}
