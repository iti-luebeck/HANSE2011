#ifndef MODULE_COMPASS_H
#define MODULE_COMPASS_H

#include <QtCore>
#include <Framework/robotmodule_mt.h>
#include "inttypes.h"

class Module_UID;

class Module_Compass : public RobotModule_MT {
    Q_OBJECT

public:
    Module_Compass(QString id, Module_UID *uid);
    ~Module_Compass();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
     * return current heading
     *
     * unit: [0..360] degree
     */
    float getHeading(void);

    float getPitch(void);

    float getRoll(void);

    void stopCalibration();
    void startCalibration();

public slots:
    void reset();
    void terminate();
//    void getPitch(float &pitch);
//    void getRoll(float &roll);
//    void getHeading(float &heading);

signals:
    void healthStatusChanged(HealthStatus data);
    void newHeading(float heading);
    void newPitch(float pitch);
    void newRoll(float roll);

       /**
         * signals for UID communication
         */
    void I2C_Read(unsigned char address, short byteCount, char* result, bool status);
    void getUIDErrorMsg(QString err);
protected:
    virtual void doHealthCheck();

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QThread thread;
    QTimer timer;

    unsigned short toShort(uint8_t high, uint8_t low);

    bool eepromRead(uint8_t addr, uint8_t &data);
    bool eepromWrite(uint8_t addr, uint8_t data);

    void setOrientation();
    void configure();

    void updateHeadingData(void);
    void updateMagData(void);
    void updateAccelData(void);
    void updateStatusRegister(void);
    void resetDevice(void);

    bool readWriteDelay(char *send_buf, int send_size,
                        char *recv_buf, int recv_size, int delay);

    void printEEPROM();

};

#endif // MODULE_COMPASS_H
