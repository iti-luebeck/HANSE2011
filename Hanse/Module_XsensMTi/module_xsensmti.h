#ifndef MODULE_XSENSMTI_H
#define MODULE_XSENSMTI_H

#include <QtCore>
#include <Framework/robotmodule.h>

class Module_Simulation;

// forward declaration. this type is never defined under win32 and
// must therefore never be used there
namespace Xsens {
    class MTi;
}

class Module_XsensMTi : public RobotModule
{
    Q_OBJECT

public:
    Module_XsensMTi(QString id, Module_Simulation *sim);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    float getHeading();
    float getHeadingIncrement();

private:
    void init();

public slots:
    void reset();
    void terminate();
    void setEnabled(bool value);

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
