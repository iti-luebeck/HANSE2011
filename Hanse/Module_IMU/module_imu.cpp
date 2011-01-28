#include "module_imu.h"
#include "imu_form.h"
#include <Module_UID/module_uid.h>
#include <Module_Simulation/module_simulation.h>

#define ADIS_REGISTER_POWER  0x02
#define ADIS_REGISTER_GYRO_X 0x04
#define ADIS_REGISTER_GYRO_Y 0x06
#define ADIS_REGISTER_GYRO_Z 0x08
#define ADIS_REGISTER_ACCEL_X 0x0A
#define ADIS_REGISTER_ACCEL_Y 0x0C
#define ADIS_REGISTER_ACCEL_Z 0x0E
#define ADIS_REGISTER_TEMP_X 0x10
#define ADIS_REGISTER_TEMP_Y 0x12
#define ADIS_REGISTER_TEMP_Z 0x14
#define ADIS_REGISTER_AUX_AD 0x16

#define ADIS_REGISTER_COMMAND_LO 0x3E

#define ADIS_COMMAND_NULL_CALIBRATION		0x01
#define ADIS_COMMAND_FACTORY_CALIBRATION	0x02
#define ADIS_COMMAND_AUX_DATA_LATCH			0x04
#define ADIS_COMMAND_FLASH_UPDATE			0x08
#define ADIS_COMMAND_PRECISION_CALIBRATION	0x10
#define ADIS_COMMAND_SOFTWARE_RESET			0x80


// Flash memory write count. (R)
#define ADIS_REGISTER_ENDURANCE 0x01

// Gyroscope bias offset factor. (R/W)
#define ADIS_REGISTER_XGYRO_OFF_HI 0x1B
#define ADIS_REGISTER_XGYRO_OFF_LO 0x1A
#define ADIS_REGISTER_YGYRO_OFF_HI 0x1D
#define ADIS_REGISTER_YGYRO_OFF_LO 0x1C
#define ADIS_REGISTER_ZGYRO_OFF_HI 0x1F
#define ADIS_REGISTER_ZGYRO_OFF_LO 0x1E

// Acceleration bias offset factor. (R/W)
#define ADIS_REGISTER_XACCL_OFF_HI 0x21
#define ADIS_REGISTER_XACCL_OFF_LO 0x20
#define ADIS_REGISTER_YACCL_OFF_HI 0x23
#define ADIS_REGISTER_YACCL_OFF_LO 0x22
#define ADIS_REGISTER_ZACCL_OFF_HI 0x25
#define ADIS_REGISTER_ZACCL_OFF_LO 0x24

// Alarm amplitude threshold. (R/W)
#define ADIS_REGISTER_ALM_MAG1_HI 0x27
#define ADIS_REGISTER_ALM_MAG1_LO 0x26
#define ADIS_REGISTER_ALM_MAG2_HI 0x29
#define ADIS_REGISTER_ALM_MAG2_LO 0x28

// Alarm sample period. (R/W)
#define ADIS_REGISTER_ALM_SMPL1_HI 0x2B
#define ADIS_REGISTER_ALM_SMPL1_LO 0x2A
#define ADIS_REGISTER_ALM_SMPL2_HI 0x2D
#define ADIS_REGISTER_ALM_SMPL2_LO 0x2C

// Alarm control. (R/W)
#define ADIS_REGISTER_ALM_CTRL_HI 0x2F
#define ADIS_REGISTER_ALM_CTRL_LO 0x2D

// Auxiliary DAC data. (R/W)
#define ADIS_REGISTER_AUX_DAC_HI 0x31
#define ADIS_REGISTER_AUX_DAC_LO 0x30

// Auxiliary digital input/output control. (R/W)
#define ADIS_REGISTER_GPIO_CTRL_HI 0x33
#define ADIS_REGISTER_GPIO_CTRL_LO 0x32

// Miscellaneous control. (R/W)
#define ADIS_REGISTER_MSC_CTRL_HI 0x35
#define ADIS_REGISTER_MSC_CTRL_LO 0x34

// Internal sample period (rate) control. (R/W)
#define ADIS_REGISTER_SMPL_PRD_HI 0x37
#define ADIS_REGISTER_SMPL_PRD_LO 0x36

// Dynamic range/digital filter control. (R/W)
#define ADIS_REGISTER_SENS_AVG_HI 0x39
#define ADIS_REGISTER_SENS_AVG_LO 0x38

// Sleep mode control. (R/W)
#define ADIS_REGISTER_SLP_CNT_HI 0x3B
#define ADIS_REGISTER_SLP_CNT_LO 0x3A

// System Status. (R)
#define ADIS_REGISTER_STATUS_HI 0x3D
#define ADIS_REGISTER_STATUS_LO 0x3C

Module_IMU::Module_IMU(QString id, Module_UID *uid, Module_Simulation *sim)
    : RobotModule(id)
{
    this->uid=uid;
    this->sim=sim;

    setDefaultValue("frequency", 10);
    setDefaultValue("ssLine", 1);
    setDefaultValue("g", 9.81);
    setDefaultValue("spiSpeed",3);
    setDefaultValue("biasComp",0);
    setDefaultValue("originAllign",0);
    setDefaultValue("smplTimeBase",0);
    setDefaultValue("smplTimeMult",1);
    setDefaultValue("filterTaps",2);
    setDefaultValue("gyroSens","300");
}

Module_IMU::~Module_IMU()
{
}

void Module_IMU::terminate()
{
//    QTimer::singleShot(0, &timer, SLOT(stop()));
    timer.stop();
    RobotModule::terminate();
}

void Module_IMU::init()
{
    timer.moveToThread(this);
    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()));

    /* connect sim */
    connect(sim,SIGNAL(newIMUData(float,float,float,float,float,float)),this,SLOT(refreshSimData(float,float,float,float,float,float)));
    connect(this,SIGNAL(requestIMU()),sim,SLOT(requestIMUSlot()));

    reset();
}

void Module_IMU::reset()
{
    RobotModule::reset();

    if (!getSettingsValue("enabled").toBool())
        return;

    if(sim->isEnabled())
    {
        timer.setInterval(250);
        QTimer::singleShot(0,&timer,SLOT(start()));
        return;
    }


    int freq = 1000/getSettingsValue("frequency").toInt();
    if (freq>0) {
        timer.setInterval(freq);
        QTimer::singleShot(0, &timer, SLOT(start()));
    } else {
        QTimer::singleShot(0, &timer, SLOT(stop()));
    }
    setHealthToOk();


    // soft reset
    writeRegister(ADIS_REGISTER_COMMAND_LO, ADIS_COMMAND_SOFTWARE_RESET);

    // set volatile registers
    configureADIS();

    doSelfTest();

    updateBiasFields();
    printRegisters();

    addData("flashCounter", readRegister(ADIS_REGISTER_ENDURANCE));

}

void Module_IMU::refreshData()
{

//    qDebug() << "adis ref THREAD ID";
//    qDebug() << QThread::currentThreadId();

    if (!getSettingsValue("enabled").toBool())
        return;

    if(sim->isEnabled())
    {
        logger->debug("request sim");
        emit requestIMU();
    }
    else
    {
    configureSPI();

    int currentGyroX;
    int currentGyroY;
    int currentGyroZ;
    int currentAccelX;
    int currentAccelY;
    int currentAccelZ;
    int gyroTempX;
    int gyroTempY;
    int gyroTempZ;

    currentGyroX = readDataRegister(ADIS_REGISTER_GYRO_X, 14);
    currentGyroY = readDataRegister(ADIS_REGISTER_GYRO_Y, 14);
    currentGyroZ = readDataRegister(ADIS_REGISTER_GYRO_Z, 14);
    currentAccelX = readDataRegister(ADIS_REGISTER_ACCEL_X, 14);
    currentAccelY = readDataRegister(ADIS_REGISTER_ACCEL_Y, 14);
    currentAccelZ = readDataRegister(ADIS_REGISTER_ACCEL_Z, 14);
    gyroTempX = readDataRegister(ADIS_REGISTER_TEMP_X, 12);
    gyroTempY = readDataRegister(ADIS_REGISTER_TEMP_Y, 12);
    gyroTempZ = readDataRegister(ADIS_REGISTER_TEMP_Z, 12);

    unsigned short voltage_raw = readRegister(ADIS_REGISTER_POWER);
    double voltage = (voltage_raw & 0x0FFF) *1.8315; // mV
    addData("voltage", voltage/1000);

    if (getHealthStatus().isHealthOk()) {

        addData("gyroX", currentGyroX * 0.07326);
        addData("gyroY", currentGyroY * 0.07326);
        addData("gyroZ", currentGyroZ * 0.07326);

        addData("accelX", currentAccelX * 0.4672 * getSettingsValue("g").toFloat() / 1000);
        addData("accelY", currentAccelY * 0.4672 * getSettingsValue("g").toFloat() / 1000);
        addData("accelZ", currentAccelZ * 0.4672 * getSettingsValue("g").toFloat() / 1000);

        addData("gyroTempX", gyroTempX * 0.1453 + 25);
        addData("gyroTempY", gyroTempY * 0.1453 + 25);
        addData("gyroTempZ", gyroTempZ * 0.1453 + 25);

        emit dataChanged(this);

    }

    }
}

void Module_IMU::refreshSimData(float accl_x, float accl_y, float accl_z, float angvel_x, float angvel_y, float angvel_z)
{
    logger->debug("new sim data");
    addData("gyroX",angvel_x);
    addData("gyroY",angvel_y);
    addData("gyroZ",angvel_z);

    addData("accelX",accl_x);
    addData("accelY",accl_y);
    addData("accelZ",accl_z);

    addData("gyroTempX",0);
    addData("gyroTempY",0);
    addData("gyroTempZ",0);

    if(getHealthStatus().isHealthOk())
        emit dataChanged(this);
}

QList<RobotModule*> Module_IMU::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    ret.append(sim);
    return ret;
}

QWidget* Module_IMU::createView(QWidget* parent)
{
    return new IMU_Form(this, parent);
}

void Module_IMU::doHealthCheck()
{

//    qDebug() << "adis health THREAD ID";
//    qDebug() << QThread::currentThreadId();

    if (!getSettingsValue("enabled").toBool())
        return;

    if(sim->isEnabled())
        return;

    // we don't check for voltage. we just want to see if the device returns
    // some sensible data to rule out cable problems (SPI will return weired
    // data with connection problems)
    unsigned short voltage_raw = readRegister(ADIS_REGISTER_POWER);
    double voltage = (voltage_raw & 0x0FFF) *1.8315; // mV

    // 4400mV is way below spec.
    if (voltage < 4400 || voltage > 5250) {
        setHealthToSick("ADIS not responding.");
        return;
    }

    short status_reg = readRegister(ADIS_REGISTER_STATUS_LO);
    status_reg &= 0xFFFE; // clear out undervoltage warning. we're aware of it.

    addData("statusReg", status_reg);
    if (status_reg != 0x0000) {
        setHealthToSick("Status Register indicating an error: register content=0x" +QString::number(status_reg,16));
        // TODO: translate error bits
        return;
    }

    setHealthToOk();
}

void Module_IMU::doSelfTest()
{
    // start self test
    writeRegister(ADIS_REGISTER_MSC_CTRL_HI, 0x04);

    // test should take approx. 35 ms
    // while (readRegister(ADIS_REGISTER_STATUS_HI) & 0x040)
    msleep(35);

    short checkResult = readRegister(ADIS_REGISTER_STATUS_HI);
    if (checkResult != 0x0000) {
        setHealthToSick("Self-test failed: status=0x" +QString::number(checkResult,16));
    }
}

int Module_IMU::readDataRegister(uint8_t reg, int bits)
{
        unsigned int data = readRegister(reg);

// We check for health independently anyway.
//        if (data & 0x4000) {
//                setHealthToSick("Return data has the EA flag set.");
//        }

        if (bits==12)
            return shortToInteger(data,0x0FFF, bits);
        else
            return shortToInteger(data,0x3FFF, bits);
}

void Module_IMU::configureSPI()
{
        uid->SPI_SetPOL(true);
        uid->SPI_SetPHA(true);
        uid->SPI_Speed(getSettingsValue("spiSpeed").toInt()); // XXX: 14.67Mhz/X < 2Mhz
}

void Module_IMU::configureADIS()
{
    configureSPI();

    short filterTaps = getSettingsValue("filterTaps").toInt();

    short sens_avg_soll = 0;
    sens_avg_soll |= filterTaps;

    QString gyroSens = getSettingsValue("gyroSens").toString();
    if (gyroSens == "75") {
        sens_avg_soll |= 0x0100;
    } else if (gyroSens == "150") {
        sens_avg_soll |= 0x0200;
    } else if (gyroSens == "300") {
        sens_avg_soll |= 0x0400;
    }

    writeFullRegister(ADIS_REGISTER_SENS_AVG_LO, sens_avg_soll);

    short multiplier = getSettingsValue("smplTimeMult").toInt()-1;
    short smpl_prd_soll = multiplier;
    if (getSettingsValue("smplTimeBase").toBool()) {
            smpl_prd_soll |= (1 << 6);
    } else {
            smpl_prd_soll &= ~(1 << 6);
    }

    writeFullRegister(ADIS_REGISTER_SMPL_PRD_LO, smpl_prd_soll);

    uint8_t msc_ctrl = 0;
    if (getSettingsValue("originAllign").toBool()) {
        msc_ctrl |= 0x40;
    }
    if (getSettingsValue("biasComp").toBool()) {
        msc_ctrl |= 0x80;
    }
    writeRegister(ADIS_REGISTER_MSC_CTRL_LO, msc_ctrl);
}

void Module_IMU::printRegisters()
{
        for (uint8_t addr = 0x00; addr <= 0x3F; addr += 2)
        {
 //            data["reg-"+QString::number(addr,16)] = readRegister(addr);
             logger->debug("Content at address 0x"+QString::number((short)addr,16) + ": 0x" +QString::number(readRegister(addr),16));
        }
}

/**
 * Converts an unsigned short that is read from the bus to
 * a signed integer.
 *
 * @param s - value to convert in twos-complement
 * @param mask - mask to be applied to s, if the value is not
 * 		16 bits
 * @param bits - number of bits
 */
int Module_IMU::shortToInteger(unsigned short s, unsigned short mask, int bits)
{
        int ret;

        // Check if the number is negative. If so invert
        // s, apply the mask, and add one. (twos-complement)
        if (((s >> (bits-1)) & 1) == 1)
        {
                s = ~s;
                s &= mask;
                s++;
                ret = - (int) s;
        }
        // If the number is positive we can simply apply the mask and
        // cast to int.
        else
        {
                s &= mask;
                ret = (int) s;
        }
        return ret;
}

unsigned short Module_IMU::toShort(uint8_t high, uint8_t low)
{
        return ((unsigned short)high) << 8 | ((unsigned short)low);
}

unsigned short Module_IMU::readRegister(uint8_t address)
{
    char buf_recv[] = {0x00, 0x00};
    char buf_send[] = {address, 0x00};

    bool ret = uid->SPI_Write(getSettingsValue("ssLine").toInt(), buf_send, 2);
    if (!ret)
        setHealthToSick(uid->getLastError());
    ret = uid->SPI_Read(getSettingsValue("ssLine").toInt(), 2, buf_recv);
    if (!ret)
        setHealthToSick(uid->getLastError());
    return toShort(buf_recv[0],buf_recv[1]);
}

void Module_IMU::writeFullRegister(uint8_t address_lower, unsigned short data)
{
        // write upper register first, since the SENS/AVG register should
        // be programmed in this order
        writeRegister(address_lower+1, data>>8);
        writeRegister(address_lower, data & 0x00FF);
}

void Module_IMU::writeRegister(uint8_t address, uint8_t data)
{
    char buf_recv[] = {0x00, 0x00};
    char buf_send[] = {address, data};

    // set highest bit to indicate write
    buf_send[0] |= 0x80;

    bool ret = uid->SPI_Write(getSettingsValue("ssLine").toInt(), buf_send, 2);
    if (!ret)
        setHealthToSick(uid->getLastError());
    ret = uid->SPI_Read(getSettingsValue("ssLine").toInt(), 2, buf_recv);
    if (!ret)
        setHealthToSick(uid->getLastError());
}

float Module_IMU::getGyroX(void)
{
    return getDataValue("gyroX").toFloat();
}

float Module_IMU::getGyroY(void)
{
    return getDataValue("gyroY").toFloat();
}

float Module_IMU::getGyroZ(void)
{
    return getDataValue("gyroZ").toFloat();
}

float Module_IMU::getAccelX(void)
{
    return getDataValue("accelX").toFloat();
}

float Module_IMU::getAccelY(void)
{
    return getDataValue("accelY").toFloat();
}

float Module_IMU::getAccelZ(void)
{
    return getDataValue("accelZ").toFloat();
}

void Module_IMU::doNullCalib()
{
    writeRegister(ADIS_REGISTER_COMMAND_LO, ADIS_COMMAND_NULL_CALIBRATION);
}

void Module_IMU::doPrecisionCalib()
{
    writeRegister(ADIS_REGISTER_COMMAND_LO, ADIS_COMMAND_PRECISION_CALIBRATION);
}

void Module_IMU::updateBiasFields()
{
    addData("biasGyroX", readDataRegister(ADIS_REGISTER_XGYRO_OFF_LO, 12));
    addData("biasGyroY", readDataRegister(ADIS_REGISTER_YGYRO_OFF_LO, 12));
    addData("biasGyroZ", readDataRegister(ADIS_REGISTER_ZGYRO_OFF_LO, 12));
    addData("biasAccelX", readDataRegister(ADIS_REGISTER_XACCL_OFF_LO, 12));
    addData("biasAccelY", readDataRegister(ADIS_REGISTER_YACCL_OFF_LO, 12));
    addData("biasAccelZ", readDataRegister(ADIS_REGISTER_ZACCL_OFF_LO, 12));
}
