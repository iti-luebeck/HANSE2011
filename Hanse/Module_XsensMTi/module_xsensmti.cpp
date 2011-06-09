#include "module_xsensmti.h"
#include <Module_XsensMTi/xsens_form.h>
#include <Module_Simulation/module_simulation.h>
#include <Framework/Angles.h>

#ifdef ENABLE_XSENS
#include "Module_XsensMTi/MTi/MTi.h"
#endif

Module_XsensMTi::Module_XsensMTi(QString id, Module_Simulation *sim)
        : RobotModule(id)
{
    this->sim = sim;
    setDefaultValue("updaterate", 10);
    setDefaultValue("port", "/dev/ttyUSB0");
    setDefaultValue("baudrate", 921600);
    setDefaultValue("upsidedown", true);

    connected = false;
    mti = NULL;
    timer.moveToThread(this);
    heading = 0;
    lastHeading = 0;
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

void Module_XsensMTi::setEnabled(bool value)
{
    RobotModule::setEnabled(value);

#ifdef ENABLE_XSENS
    if (!value) {
        if (mti != NULL) {
            delete(mti);
            mti = NULL;
        }
    }
#endif
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

#ifdef ENABLE_XSENS
    if (mti != NULL) {
        delete(mti);
        connected = false;
    }
#endif

    sleep(1);

#ifdef ENABLE_XSENS
    mti = new Xsens::MTi();

    if(!mti->openPort((char*)getSettingsValue("port").toString().toStdString().c_str(), getSettingsValue("baudrate").toInt())) {
        setHealthToSick("Unable to connect to the MTi");
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
            setHealthToSick("Unable to set the output mode and settings");
        } else {
            connected = true;
            setHealthToOk();
        }
    }
#endif
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
#ifdef ENABLE_XSENS
        if (mti != NULL) {
            if (getSettingsValue("upsidedown").toBool()) {
                addData("yaw", -Angles::deg2deg(mti->yaw()));
            } else {
                addData("yaw", Angles::deg2deg(mti->yaw()));
            }
            addData("pitch", Angles::deg2deg(mti->pitch()));
            addData("roll", Angles::deg2deg(mti->roll()));

            if (getHealthStatus().isHealthOk()) {
                emit dataChanged(this);
            }
        }
#endif
    }
}

void Module_XsensMTi::refreshSimData(float angle_yaw, float angle_pitch, float angle_roll)
{
    if (getSettingsValue("upsidedown").toBool()) {
        addData("yaw", -Angles::deg2deg(angle_yaw));
    } else {
        addData("yaw", Angles::deg2deg(angle_yaw));
    }
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
    increment = Angles::deg2deg(getDataValue("yaw").toFloat() - lastHeading);

    lastHeading = getDataValue("yaw").toFloat();
    return increment;
}

