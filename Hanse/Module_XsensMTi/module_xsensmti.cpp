#include "module_xsensmti.h"
#include <Module_XsensMTi/xsens_form.h>
#include <Module_Simulation/module_simulation.h>
#include <Framework/Angles.h>

Module_XsensMTi::Module_XsensMTi(QString id, Module_Simulation *sim)
        : RobotModule(id)
{
    this->sim = sim;
    setDefaultValue("updaterate", 10);
    setDefaultValue("port", "/dev/ttyUSB0");
    setDefaultValue("baudrate", 921600);

    connected = false;
    mti = NULL;
    timer.moveToThread(this);
    heading = 0;
}

Module_XsensMTi::~Module_XsensMTi() {
    terminate();

    if (mti != NULL) {
        delete(mti);
    }
}

void Module_XsensMTi::init()
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(refreshData()));

    /* for simulation */
    connect(sim, SIGNAL(newAngleData(float,float,float)), this, SLOT(refreshSimData(float,float,float)));
    connect(this, SIGNAL(requestAngles()), sim, SLOT(requestAnglesSlot()));

    reset();
}

void Module_XsensMTi::terminate()
{
    QTimer::singleShot(0, &timer, SLOT(stop()));
    RobotModule::terminate();
}

void Module_XsensMTi::reset()
{
    RobotModule::reset();

    if (!getSettingsValue("enabled").toBool())
        return;

    int freq = 1000 / getSettingsValue("updaterate").toInt();
    if (freq > 0) {
        timer.setInterval(freq);
        timer.start();
    } else {
        timer.stop();
    }

    if(sim->isEnabled())
        return;

    if (mti != NULL) {
        delete(mti);
        connected = false;
    }

    sleep(1);

    mti = new Xsens::MTi();

    if(!mti->openPort((char*)getSettingsValue("port").toString().toStdString().c_str(), getSettingsValue("baudrate").toInt())) {
        qDebug("MTi -- Unable to connect to the MTi.");
    } else {
        Xsens::MTi::outputMode outputMode;
        outputMode.temperatureData = false;
        outputMode.calibratedData = true;
        outputMode.orientationData = true;
        outputMode.auxiliaryData = false;
        outputMode.positionData = false;
        outputMode.velocityData = false;
        outputMode.statusData = false;
        outputMode.rawGPSData = false;
        outputMode.rawInertialData = false;

        Xsens::MTi::outputSettings outputSettings;
        outputSettings.timeStamp = false;
        outputSettings.orientationMode = Xsens::EulerAngles;

        if(!mti->setOutputModeAndSettings(outputMode, outputSettings, 1000)) {
            qDebug("MTi -- Unable to set the output mode and settings.");
        } else {
            connected = true;
        }
    }
}

QList<RobotModule*> Module_XsensMTi::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* Module_XsensMTi::createView(QWidget* parent)
{
    return new Xsens_Form(this, parent);
}

void Module_XsensMTi::doHealthCheck() {

}

void Module_XsensMTi::refreshData()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    if(sim->isEnabled()) {
        emit requestAngles();
    }
    else {
        addData("yaw", Angles::pi2deg(mti->yaw()));
        addData("pitch", Angles::pi2deg(mti->pitch()));
        addData("roll", Angles::pi2deg(mti->roll()));

        if (getHealthStatus().isHealthOk()) {
            emit dataChanged(this);
        }
    }
}

void Module_XsensMTi::refreshSimData(float angle_yaw, float angle_pitch, float angle_roll)
{
    addData("yaw", Angles::deg2deg(angle_yaw));
    addData("pitch", Angles::deg2deg(angle_pitch));
    addData("roll", Angles::deg2deg(angle_roll));
    if (getHealthStatus().isHealthOk()) {
        emit dataChanged(this);
    }
}

float Module_XsensMTi::getHeading() {
    return getDataValue("yaw").toFloat();
}

float Module_XsensMTi::getHeadingIncrement() {
    float increment = 0;
    if (sim->isEnabled()) {
        increment = Angles::deg2deg(getDataValue("yaw").toFloat() - lastHeading);
    } else {
        addData("yaw", Angles::pi2deg(mti->yaw()));
        addData("pitch", Angles::pi2deg(mti->pitch()));
        addData("roll", Angles::pi2deg(mti->roll()));

        increment = Angles::deg2deg(getDataValue("yaw").toFloat() - lastHeading);
    }

    lastHeading = getDataValue("yaw").toFloat();
    return increment;
}

