#ifndef MODULE_IMU_H
#define MODULE_IMU_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;
class Module_Simulation;

class Module_IMU : public RobotModule {
    Q_OBJECT

    friend class IMU_Form;

public:
    Module_IMU(QString id, Module_UID *uid, Module_Simulation *sim);
    ~Module_IMU();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

    /**
     * Return the acceleration along the three axis.
     * (these are the axis of the ADIS sensor, not the boat)
     */
    float getAccelX(void);
    float getAccelY(void);
    float getAccelZ(void);

    /**
     * Return the gyro angle velocities along the three axis.
     * (these are the axis of the ADIS sensor, not the boat)
     */
    float getGyroX(void);
    float getGyroY(void);
    float getGyroZ(void);



public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);
    void requestIMU();

protected slots:
    virtual void doHealthCheck();

private slots:
    void refreshData();    
    void doPrecisionCalib();
//    void refresh();
    void doSelfTest();
    void doNullCalib();
    void updateBiasFields();
    void refreshSimData(float accl_x, float accl_y, float accl_z, float angvel_x,float angvel_y, float angvel_z);

private:
    Module_UID *uid;
    Module_Simulation *sim;
    QTimer timer;

    bool readRegister(unsigned char reg, int size, unsigned char *ret_buf);

    void configureSPI();
    void configureADIS();

    void printRegisters();

    unsigned short toShort(uint8_t, uint8_t);
    int shortToInteger(unsigned short, unsigned short, int);

    unsigned short readRegister(uint8_t);
    void writeRegister(uint8_t,uint8_t);
    void writeFullRegister(uint8_t,unsigned short);

    void setInternalSampleRate(unsigned short, unsigned short);
    void setFilterSettings(unsigned short, unsigned short);

    int readDataRegister(uint8_t reg, int bits);

    void init();
    /**
     * Read out new data from the sensors.
     */
//    void refresh();
//
//    void doSelfTest();
//
//    void doNullCalib();
//
//    void doPrecisionCalib();
//
//    void updateBiasFields();

};

#endif // MODULE_IMU_H
