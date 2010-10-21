#ifndef TestMTBEHAVIOUR_H
#define TestMTBEHAVIOUR_H

#include <Framework/robotbehaviour_mt.h>

#include <Behaviour_TestMT/testmtform.h>
#include <Module_Compass/module_compass.h>
#include <Module_IMU/module_imu.h>
#include <Module_PressureSensor/module_pressuresensor.h>
//#include <Framework/robotmodule_mt.h>

class TestMTForm;

class Behaviour_TestMT : public RobotBehaviour_MT
{
    Q_OBJECT
public:
    Behaviour_TestMT(QString id, Module_Compass *compass, Module_IMU *adis, Module_PressureSensor *pressure);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

        bool isActive();


private:
        Module_Compass* compass;
        Module_IMU* adis;
        Module_PressureSensor* pressure;
public slots:
        void start();
        void stop();
        void reset();

private slots:

};


#endif // TestMTBEHAVIOUR_H
