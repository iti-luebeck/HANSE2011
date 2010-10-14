#ifndef MODULE_SIMULATION_H
#define MODULE_SIMULATION_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;

class Module_Simulation : public RobotModule {
    Q_OBJECT

public:
    Module_Simulation(QString id, Module_UID *uid);
    ~Module_Simulation();

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

protected:
    virtual void doHealthCheck();

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;
    QThread thread;
    unsigned int counter;

    void readPressure();
    void readTemperature();

    bool readRegister(unsigned char reg, int size, char *ret_buf);

};

#endif // MODULE_SIMULATION_H
