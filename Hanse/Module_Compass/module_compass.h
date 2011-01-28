#ifndef MODULE_COMPASS_H
#define MODULE_COMPASS_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;
class Module_Simulation;

class Module_Compass : public RobotModule {
    Q_OBJECT

public:
    Module_Compass(QString id, Module_UID *uid,Module_Simulation *sim);
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


signals:
    void healthStatusChanged(HealthStatus data);
    void requestAngles();

protected slots:
    virtual void doHealthCheck();

private slots:
    void refreshData();
    void refreshSimData(float angle_yaw, float angle_pitch, float angle_roll);

private:
    Module_UID *uid;
    Module_Simulation *sim;
    QTimer timer;

    void init();
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
