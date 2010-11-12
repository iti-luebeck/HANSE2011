#ifndef MODULE_PRESSURESENSOR_H
#define MODULE_PRESSURESENSOR_H

#include <QtCore>
#include <Framework/robotmodule_mt.h>
#include "inttypes.h"

class Module_UID;

class Module_PressureSensor : public RobotModule_MT {
    Q_OBJECT

public:
    Module_PressureSensor(QString id, Module_UID *uid);
    ~Module_PressureSensor();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * Return depth below the surface in meters.
      *
      * Range: 0 to infinity
      *
      */
    float getDepth();

    /**
      * Return temperature of the pressure sensor in degree celsius
      *
      */
    float getTemperature();

public slots:
    void reset();
    void terminate();


signals:
    void healthStatusChanged(HealthStatus data);

    /**
      * This signal is emmited whenever there a new messurement is available.
      *
      */
    void newDepthData(float depth);

protected slots:
    virtual void doHealthCheck();

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;
    unsigned int counter;

    void readPressure();
    void readTemperature();

    bool readRegister(unsigned char reg, int size, char *ret_buf);

};

#endif // MODULE_PRESSURESENSOR_H
