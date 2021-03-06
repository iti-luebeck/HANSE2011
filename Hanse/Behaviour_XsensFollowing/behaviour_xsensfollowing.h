#ifndef XSENSFOLLOWINGBEHAVIOUR_H
#define XSENSFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_XsensFollowing/xsensfollowingform.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

#define STATE_RUNNING  "Running"
#define STATE_FINISHED "Finished"

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
        QString behavState;
        void stopOnXsensError();

        float targetHeading;
        bool active;
        int turnCounter;

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
        void newXsensState(QString state);
};


#endif // XSENSFOLLOWINGBEHAVIOUR_H
