#ifndef MODULE_SIMULATION_H
#define MODULE_SIMULATION_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "inttypes.h"

class Module_UID;

class Module_Simulation : public RobotModule {
    Q_OBJECT

public:
    Module_Simulation(QString id);
    ~Module_Simulation();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

protected:
    virtual void doHealthCheck();

private slots:

private:
    QTimer timer;
    QThread thread;
    unsigned int counter;
};

#endif // MODULE_SIMULATION_H
