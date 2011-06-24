#ifndef XSENSFOLLOWINGBEHAVIOUR_H
#define XSENSFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_XsensFollowing/xsensfollowingform.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

class XsensFollwingForm;

class Behaviour_XsensFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_XsensFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_XsensMTi *xsens);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

        bool isActive();



private:
        Module_XsensMTi* xsens;
        Module_ThrusterControlLoop* tcl;
        QTimer timer;
        QTimer turnTimer;
        void stopOnXsensError();

        float targetHeading;
        bool active;

public slots:
        void startBehaviour();
        void stop();
        void reset();
        void refreshHeading();
        void init();
        void controlEnabledChanged(bool);

private slots:
        void controlLoop();
        void turnNinety();

signals:
        void newAngularSpeed(float angspeed);
        void newForwardSpeed(float ffspeed);
        void startControlTimer();
        void startTurnTimer();
};


#endif // XSENSFOLLOWINGBEHAVIOUR_H
