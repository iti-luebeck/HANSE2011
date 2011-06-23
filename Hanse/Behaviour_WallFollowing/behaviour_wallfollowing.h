#ifndef WALLFOLLOWINGBEHAVIOUR_H
#define WALLFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_EchoSounder/module_echosounder.h>

#define FLAG_SOUNDER_RIGHT true

class WallFollowingForm;
class Module_ThrusterControlLoop;
class Module_Simulation;
class Module_XsensMTi;

class Behaviour_WallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo, Module_XsensMTi* x);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    bool isActive();

    bool echoTest;
    bool t90dt90;
    float diff;

    QTimer *echoControlTimer;
    Module_EchoSounder *echo;
    Module_XsensMTi* xsens;

private:
    //QTimer timer;
    void init();
    QString wallCase;
    void controlWallFollow();


    // void timerSlotExecute();

    Module_ThrusterControlLoop * tcl;

    // int timerTime;
    bool running;
    bool startPhase;

    float avgDistance;
    float distanceInput;
    float angSpeed;
    float fwdSpeed;
    float corridorWidth;
    void timerSlotExecute();
    int wallTime;
    double initialHeading;

public slots:
    void updateFromSettings();
    void startBehaviour();
    void stop();
    void reset();
    void terminate();
    void stopOnEchoError();

    // Module_EchoSounder -> Behaviour_WallFollowing
    void newWallBehaviourData(const EchoReturnData data, float avgDistance);

    void testEchoModule();
    void controlEnabledChanged(bool);

    void turn90One();
    void drive();
    void turn90Two();

    void controlWallFollowThruster();

signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);

    void startControlTimer();

    // Behaviour_WallFollowing -> WallFollowingForm
    void newWallUiData(const EchoReturnData data, float avgDistance);
    void updateWallCase(QString caseW);
    void updateUi();


};


#endif // WallFollowingBEHAVIOUR_H
