#include "module_adc.h"
#include <Module_UID/module_uid.h>
#include <QtGui>
#include "Module_ADC/adc_form.h"

Module_ADC::Module_ADC(QString id, Module_UID *uid)
    : RobotModule(id)
{

    this->uid=uid;

    setDefaultValue("i2cAddress", 0x48);
    setDefaultValue("frequency", 1);
    setDefaultValue("Vagnd", 0);
    setDefaultValue("Vref", 5);
    setDefaultValue("waterFilter",5);
    setDefaultValue("filtersize",5);
        timer.moveToThread(this);
}

Module_ADC::~Module_ADC()
{
}

void Module_ADC::terminate()
{
    QTimer::singleShot(0, &timer, SLOT(stop()));
    RobotModule::terminate();
}

void Module_ADC::init()
{
//    timer.moveToThread(this);
    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()));
    reset();

}

void Module_ADC::reset()
{
    RobotModule::reset();
    initFilter();
    int freq = 1000/getSettingsValue("frequency").toInt();
    if (freq>0) {
        timer.setInterval(freq);
//        QTimer::singleShot(0, &timer, SLOT(start()));
        timer.start();
    } else {
        timer.stop();
//        QTimer::singleShot(0, &timer, SLOT(stop()));
    }

    if (!getSettingsValue("enabled").toBool())
        return;

}

void Module_ADC::refreshData()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    float value;
    if (readChannel(0, &value))
        addData("channel0",value);
    if (readChannel(1, &value))
        addData("channel1",value);
    if (readChannel(2, &value))
        addData("channel2",value);
    if (readChannel(3, &value))
        addData("channel3",value);

}

QList<RobotModule*> Module_ADC::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    return ret;
}

QWidget* Module_ADC::createView(QWidget* parent)
{
    return new ADC_Form(this, parent);
}

void Module_ADC::initFilter()
{
    waterFilter.clear();
    voltage1.clear();
    voltage2.clear();
    voltage3.clear();
    for(int i = 0; i < getSettingsValue("filtersize").toInt();i++)
    {
        waterFilter.append(5.0);
        voltage1.append(5.0);
        voltage2.append(5.0);
        voltage3.append(5.0);
    }
}

void Module_ADC::doHealthCheck()
{
    if(waterFilter.size() == 0)
        initFilter();
    /*there are two adc converters installed, but only adc0
     * checks for voltage and water
     */
    if(this->getId() == "adc0")
    {
        /* check for water and voltage level */
        waterFilter.pop_front();
        waterFilter.append(getDataValue("channel0").toFloat());

        voltage1.pop_front();
        voltage2.pop_front();
        voltage3.pop_front();
        voltage1.append(getDataValue("channel1").toFloat());
        voltage2.append(getDataValue("channel2").toFloat());
        voltage3.append(getDataValue("channel3").toFloat());
        float med1 = 0.0;
        float med2 = 0.0;
        float med3 = 0.0;
        float med = 0.0;
        for(int i = 0; i < waterFilter.size(); i++)
        {
            med = med + waterFilter.at(i);
            med1 = med1 + voltage1.at(i);
            med2 = med2 + voltage2.at(i);
            med3 = med3 + voltage3.at(i);
        }
        med = med / waterFilter.size();
        med1 = med1 / waterFilter.size();
        med2 = med2 / waterFilter.size();
        med3 = med3 / waterFilter.size();
        if(med < 2.1)
        {
           emit emergencyStop();
           setHealthToSick("Water Inside!");
        }
        if(med1 < 3.5 || med2 < 3.5 || med3 < 3.5)
        {
            emit emergencyStop();
            setHealthToSick("low voltage");
        }
        else
            setHealthToOk();
        addData("Mchannel0m",med);
        addData("Mchannel1m",med1);
        addData("Mchannel2m",med2);
        addData("Mchannel3m",med3);

    }
}

bool Module_ADC::readChannel(int channel, float *value)
{
    unsigned char address = getSettingsValue("i2cAddress").toInt();
    float Vref = getSettingsValue("Vref").toFloat();
    float Vagnd = getSettingsValue("Vagnd").toFloat();
    char readBuffer[2];

    if (!uid->I2C_ReadRegisters(address, channel, 2, readBuffer)) {
        setHealthToSick(uid->getLastError());
        return false;
    }
    *value = Vagnd + (Vref-Vagnd)/256*((unsigned char)readBuffer[1]);
    return true;
}
