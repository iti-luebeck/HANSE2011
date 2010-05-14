#include "module_imu.h"
#include "imu_form.h"
#include <Module_UID/module_uid.h>

// Slave-select PIN auf dem UID
#define UID_IO_NUMBER 1

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

Module_IMU::Module_IMU(QString id, Module_UID *uid)
    : RobotModule(id)
{
    this->uid=uid;

    setDefaultValue("frequency", 10);
    setDefaultValue("ssLine", 1);

    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()));

    reset();
}

Module_IMU::~Module_IMU()
{
}

void Module_IMU::terminate()
{
    RobotModule::terminate();
    timer.stop();
}

void Module_IMU::reset()
{
    RobotModule::reset();

    if (!getSettings().value("enabled").toBool())
        return;

    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0)
        timer.start(freq);
    else
        timer.stop();

}

void Module_IMU::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    configureSPI();

    int currentGyroX;
    int currentGyroY;
    int currentGyroZ;
    int currentAccelX;
    int currentAccelY;
    int currentAccelZ;

    // TODO: error handling

    readDataRegister(ADIS_REGISTER_GYRO_X, &currentGyroX);
    readDataRegister(ADIS_REGISTER_GYRO_Y, &currentGyroY);
    readDataRegister(ADIS_REGISTER_GYRO_Z, &currentGyroZ);
    readDataRegister(ADIS_REGISTER_ACCEL_X, &currentAccelX);
    readDataRegister(ADIS_REGISTER_ACCEL_Y, &currentAccelY);
    readDataRegister(ADIS_REGISTER_ACCEL_Z, &currentAccelZ);

    // TODO: convert them

    data["gyroX"] = currentGyroX;
    data["gyroY"] = currentGyroY;
    data["gyroZ"] = currentGyroZ;

    data["accelX"] = currentAccelX;
    data["accelY"] = currentAccelY;
    data["accelZ"] = currentAccelZ;

    emit dataChanged(this);
}

QList<RobotModule*> Module_IMU::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    return ret;
}

QWidget* Module_IMU::createView(QWidget* parent)
{
    return new IMU_Form(this, parent);
}

void Module_IMU::doHealthCheck()
{
    if (!getSettings().value("enabled").toBool())
        return;

    printRegisters();

    short dynRange_ist = readRegister(0x38);
    short dynRange_soll = 0x0202;

    if (dynRange_ist != dynRange_soll)
    {
            setHealthToSick("SENS/AVG register has bad content: 0x" +QString::number(dynRange_ist,16));
            return;
    }

    // reset any existing offsets
//	writeFullRegister(ADIS_REGISTER_XACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_YACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_ZACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_XGYRO_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_YGYRO_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_ZGYRO_OFF_LO, 0);

    setInternalSampleRate(1,0); // 18ms sample period (TODO: higher freq. needed?)
    setFilterSettings(0x02, 0x01); // medium sensitivity, small filter(?)

    // enable linear acceleation bios compensation and
    // linear acceleration origin alignment
    //writeRegister(ADIS_REGISTER_MSC_CTRL_LO, 0x80 | 0x40);

    // start self test
    //writeRegister(ADIS_REGISTER_MSC_CTRL_HI, 0x04);

    // test should take approx. 35 ms
    // TODO: SLEEP
//        while (readRegister(ADIS_REGISTER_STATUS_HI) & 0x040)
//                Sleep(5);

//    short checkResult = readRegister(ADIS_REGISTER_STATUS_HI);
//    if (checkResult != 0x0000) {
//            logger->error("Self-test failed: status=" +QString::number(checkResult));
//            return;
//    }

    short status_reg = readRegister(ADIS_REGISTER_STATUS_HI);
    if (status_reg != 0x0000) {
        logger->error("Status Register indicating an error: register content=" +QString::number(status_reg));
            return;
    }

    // we don't check for voltage. we just want to see if the device returns
    // some sensible data to rule out cable problems (SPI will return weired
    // data with connection problems)
    unsigned short voltage_raw = readRegister(ADIS_REGISTER_POWER);
    double voltage = (voltage_raw & 0x0FFF) *1.8315; // mV

    if (voltage < 4750 || voltage > 5250) {
            logger->error("Measured voltage: "+QString::number(voltage));
            logger->error("Could not read from ADIS (not connected?)");
            return;
    }

}

void Module_IMU::readDataRegister(uint8_t reg, int* target)
{
        unsigned int data = readRegister(reg);

        if (data & 0x4000) {
                logger->error("Return data has the EA flag set.");
        }

        if (data & 0x8000) {
                *target = shortToInteger(data,0x3FFF, 14);
        }
}

void Module_IMU::configureSPI()
{
        uid->getUID()->SPI_SetPOL(true);
        uid->getUID()->SPI_SetPHA(false);
        uid->getUID()->SPI_Speed(6); // 14.67Mhz/64 =~ 250khz
}

void Module_IMU::configureADIS()
{
//        short dynRange_ist = readRegister(0x38);
//        short dynRange_soll = 0x0202;
//
//        if (dynRange_ist != dynRange_soll)
//        {
//                logger->error("SENS/AVG register has bad content: 0x" +QString::number(dynRange_ist,16));
//        }

        // reset any existing offsets
//	writeFullRegister(ADIS_REGISTER_XACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_YACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_ZACCL_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_XGYRO_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_YGYRO_OFF_LO, 0);
//	writeFullRegister(ADIS_REGISTER_ZGYRO_OFF_LO, 0);

        setInternalSampleRate(1,0); // 18ms sample period (TODO: higher freq. needed?)
        setFilterSettings(0x02, 0x01); // medium sensitivity, small filter(?)

        // enable linear acceleation bios compensation and
        // linear acceleration origin alignment
        writeRegister(ADIS_REGISTER_MSC_CTRL_LO, 0x80 | 0x40);

        // start self test
//        writeRegister(ADIS_REGISTER_MSC_CTRL_HI, 0x04);

        // test should take approx. 35 ms
        // TODO: SLEEP
//        while (readRegister(ADIS_REGISTER_STATUS_HI) & 0x040)
//                Sleep(5);

//        short checkResult = readRegister(ADIS_REGISTER_STATUS_HI);
//        if (checkResult != 0x0000) {
//                logger->error("Self-test failed: status=" +QString::number(checkResult));
//                return;
//        }
}

void Module_IMU::printRegisters()
{
        for (uint8_t addr = 0x00; addr <= 0x3F; addr += 2)
        {
             data["reg-"+QString::number(addr,16)] = readRegister(addr);
//                logger->debug("Content at address 0x"+QString::number((short)addr,16) + ": 0x" +QString::number(data,16));
        }
}


/**
 * Sets the filter settings. The filter is a Bartlett Window.
 *
 * @param range 0x04 for +-300°/s
 * 		0x02 for +-150°/s
 * 		0x01 for +-75°/s
 * @param tapSettings determines the number of filter taps 2^tapSettings.
 * Should be >= 0x02 for 150°/s and >= 0x04 for 75°/s.
 */
void Module_IMU::setFilterSettings(unsigned short range, unsigned short tapSettings)
{
        unsigned short value = (range << 8) | (tapSettings & 0xFF);

        writeFullRegister(ADIS_REGISTER_SENS_AVG_LO, value);
}

/**
 * Sets the internal sample rate. If SMPL_PRD > 0x09, only the normal
 * SPI mode with a rate up to 300 kHz can be used. Otherwise, up to
 * 2 MHz is possible.
 * The sampling rate calulates as follows:
 *  T_S = T_B * (N_S + 1)
 * where T_B = 0.61035 ms or 18.921 ms and N_S is a 6 bit value.
 *
 * @param T_B - 0 for 0.61035 ms, 1 for 18.921 ms
 * @param N_S - 6 bit value
 */
void Module_IMU::setInternalSampleRate(unsigned short T_B, unsigned short N_S)
{
        if (T_B) {
                N_S |= (1 << 6);
        } else {
                N_S &= ~(1 << 6);
        }

        writeFullRegister(ADIS_REGISTER_SMPL_PRD_LO, N_S);
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
    uint8_t buf_recv[] = {0x00, 0x00};
    uint8_t buf_send[] = {address, 0x00};

    uid->getUID()->SPI_Write(UID_IO_NUMBER, buf_send, 2);
    uid->getUID()->SPI_Read(UID_IO_NUMBER, 2, buf_recv);
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
    uint8_t buf_recv[] = {0x00, 0x00};
    uint8_t buf_send[] = {address, data};

    // set highest bit to indicate write
    buf_send[0] |= 0x80;

    uid->getUID()->SPI_Write(UID_IO_NUMBER, buf_send, 2);
    uid->getUID()->SPI_Read(UID_IO_NUMBER, 2, buf_recv);
}

float Module_IMU::getGyroX(void)
{
    return data["gyroX"].toFloat();
}

float Module_IMU::getGyroY(void)
{
        return data["gyroY"].toFloat();
}

float Module_IMU::getGyroZ(void)
{
        return data["gyroZ"].toFloat();
}

float Module_IMU::getAccelX(void)
{
        return data["accelX"].toFloat();
}

float Module_IMU::getAccelY(void)
{
        return data["accelY"].toFloat();
}

float Module_IMU::getAccelZ(void)
{
        return data["accelZ"].toFloat();
}

signed int Module_IMU::getGyroTempX(void)
{
        return shortToInteger(readRegister(ADIS_REGISTER_TEMP_X),0x0FFF, 12);
}

signed int Module_IMU::getGyroTempY(void)
{
        return shortToInteger(readRegister(ADIS_REGISTER_TEMP_Y),0x0FFF, 12);
}

signed int Module_IMU::getGyroTempZ(void)
{
        return shortToInteger(readRegister(ADIS_REGISTER_TEMP_Z),0x0FFF, 12);
}
