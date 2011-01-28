#ifndef CompassFollowingBEHAVIOUR_H
#define CompassFollowingBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_CompassFollowing/compassfollowingform.h>
#include <Module_Compass/module_compass.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

class CompassFollwingForm;

class Behaviour_CompassFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_CompassFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Compass *compass);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

        bool isActive();


private:
        Module_Compass* compass;
        Module_ThrusterControlLoop* tcl;
        QTimer *timer;
        QTimer *turnTimer;
        bool turning;
        void stopOnCompassError();
public slots:
        void start();
        void stop();
        void reset();
        void refreshHeading();

private slots:
        void controlLoop();
        void turnNinety();

signals:
        void newAngularSpeed(float angspeed);
        void newForwardSpeed(float ffspeed);
        void startControlTimer();
        void startTurnTimer();
};


#endif // CompassFollowingBEHAVIOUR_H
