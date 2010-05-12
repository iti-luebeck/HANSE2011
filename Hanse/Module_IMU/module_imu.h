#ifndef MODULE_IMU_H
#define MODULE_IMU_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;

class Module_IMU : public RobotModule {
    Q_OBJECT

public:
    Module_IMU(QString id, Module_UID *uid);
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

    /**
     * Read out new data from the sensors.
     */
    void refresh();


public slots:
    void reset();
    void terminate();


signals:
    void healthStatusChanged(HealthStatus data);

protected:
    virtual void doHealthCheck();

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;

    bool readRegister(unsigned char reg, int size, unsigned char *ret_buf);

    void init();
    void configureSPI();
    void configureADIS();

    void printRegisters();
    //void checkADIS();

    signed int getGyroTempX(void);
    signed int getGyroTempY(void);
    signed int getGyroTempZ(void);

    unsigned short toShort(uint8_t, uint8_t);
    int shortToInteger(unsigned short, unsigned short, int);

    unsigned short readRegister(uint8_t);
    void writeRegister(uint8_t,uint8_t);
    void writeFullRegister(uint8_t,unsigned short);

    void setInternalSampleRate(unsigned short, unsigned short);
    void setFilterSettings(unsigned short, unsigned short);

    void readDataRegister(uint8_t reg, int* target);
};

#endif // MODULE_IMU_H
