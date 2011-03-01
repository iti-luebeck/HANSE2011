#ifndef WALLFOLLOWINGBEHAVIOUR_H
#define WALLFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_Compass/module_compass.h>
#include <Module_IMU/module_imu.h>
#include <Module_EchoSounder/module_echosounder.h>
//#include <Framework/robotmodule.h>

class WallFollowingForm;

class Behaviour_WallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_WallFollowing(QString id, Module_EchoSounder *echo);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

        bool isActive();


private:
        Module_EchoSounder* echo;
public slots:
        void startBehaviour();
        void stop();
        void reset();

private slots:

};


#endif // WallFollowingBEHAVIOUR_H
