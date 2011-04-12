#ifndef MODULE_XSENSMTI_H
#define MODULE_XSENSMTI_H

#include <QtCore>
#include <Framework/robotmodule.h>
#include "Module_XsensMTi/MTi/MTi.h"

class Module_Simulation;

class Module_XsensMTi : public RobotModule
{
    Q_OBJECT

public:
    Module_XsensMTi(QString id, Module_Simulation *sim);
    ~Module_XsensMTi();

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    float getHeading();
    float getHeadingIncrement();

private:
    void init();

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);
    void requestAngles();

protected slots:
    void doHealthCheck();

private slots:
    void refreshData();
    void refreshSimData(float angle_yaw, float angle_pitch, float angle_roll);

private:
    Module_Simulation *sim;
    QTimer timer;
    bool connected;

    Xsens::MTi *mti;

    float heading;
    float lastHeading;
};

#endif // MODULE_XSENSMTI_H
