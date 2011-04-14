#include "module_xsensmti.h"
#include <Module_XsensMTi/xsens_form.h>
#include <Module_Simulation/module_simulation.h>

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
        addData("yaw", 180 * mti->yaw() / M_PI);
        addData("pitch", 180 * mti->pitch() / M_PI);
        addData("roll", 180 * mti->roll() / M_PI);

        if (getHealthStatus().isHealthOk()) {
            emit dataChanged(this);
        }
    }
}

void Module_XsensMTi::refreshSimData(float angle_yaw, float angle_pitch, float angle_roll)
{
    addData("yaw", angle_yaw);
    addData("pitch", angle_pitch);
    addData("roll", angle_roll);
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
        increment = getDataValue("yaw").toFloat() - lastHeading;
    } else {
        addData("yaw", 180 * mti->yaw() / M_PI);
        addData("pitch", 180 * mti->pitch() / M_PI);
        addData("roll", 180 * mti->roll() / M_PI);

        increment = getDataValue("yaw").toFloat() - lastHeading;
    }
    if (increment > 180) {
        increment -= 360;
    } else if (increment <= -180) {
        increment += 360;
    }

    lastHeading = getDataValue("yaw").toFloat();
    return increment;
}

