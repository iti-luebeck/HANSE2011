#ifndef WALLFOLLOWINGBEHAVIOUR_H
#define WALLFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_EchoSounder/module_echosounder.h>

class WallFollowingForm;
class Module_ThrusterControlLoop;
class Module_Simulation;

class Behaviour_WallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    bool isActive();

    bool echoTest;

    int badDataCount;

    QTimer *echoControlTimer;
    Module_EchoSounder *echo;

private:
    //QTimer timer;
    void init();
    QString wallCase;
    QString tempAs;
    void controlWallFollow();


    // void timerSlotExecute();

    Module_ThrusterControlLoop * tcl;

    // int timerTime;
    bool running;

    float avgDistance;
    float distanceInput;
    float angSpeed;
    float fwdSpeed;
    float corridorWidth;
    void timerSlotExecute();
    int wallTime;

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
