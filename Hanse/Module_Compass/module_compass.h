#ifndef MODULE_COMPASS_H
#define MODULE_COMPASS_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;

class Module_Compass : public RobotModule {
    Q_OBJECT

    friend class Compass_Form;

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

    unsigned short toShort(uint8_t high, uint8_t low);

    bool eepromRead(uint8_t addr, uint8_t &data);
    bool eepromWrite(uint8_t addr, uint8_t data);

    void setOrientation();
    void stopCalibration();
    void startCalibration();

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
