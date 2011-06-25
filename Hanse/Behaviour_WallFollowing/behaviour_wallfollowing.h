#ifndef WALLFOLLOWINGBEHAVIOUR_H
#define WALLFOLLOWINGBEHAVIOUR_H

#include <Framework/robotbehaviour.h>
#include <Module_EchoSounder/module_echosounder.h>

class WallFollowingForm;
class Module_ThrusterControlLoop;
class Module_Simulation;
class Module_XsensMTi;

// Follow wall right to hanse
#define FLAG_SOUNDER_RIGHT      true

// Behaviour states
#define BEHAV_BEHAVIOUR_START   "Behaviour start"
#define BEHAV_WALLFOLLOWING     "Control distance"
#define BEHAV_BEHAVIOUR_END     "Behaviour stopped"

// Wall states
#define WALL_NO_WALL                 "No average distance"
#define WALL_ADJUST_START            "Adjust startdistance"
#define WALL_CONTROL_WALLFOLLOW      "Average distance existing"
#define WALL_TURN_LEFT              "Turn left"
#define WALL_TURN_RIGHT             "Turn right"
#define WALL_NO_TURN             "Only forward"
#define WALL_ERROR               "Echosignal error"
#define WALL_STOP               " - Stopped -"

// Adjust distance states
#define ADJUST_IDLE             "Adjust idle"
#define ADJUST_TURN90_START     "First adjust turn 90"
#define ADJUST_DRIVE            "Adjust drive"
#define ADJUST_TURN90_END       "Second adjust turn 90"
#define ADJUST_FINISHED         "Adjust distance finished"

class Behaviour_WallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    // Methods
    Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo, Module_XsensMTi* x);
    QList<RobotModule*> getDependencies();
    QWidget* createView(QWidget *parent);
    bool isActive();


private:
    void init();
    void controlWallFollow();
    void controlThrusterEchoLeft();
    void controlThrusterEchoRight();
    void updateStates();

    // Parameters
    bool active;
    Module_EchoSounder *echo;
    Module_XsensMTi* xsens;
    Module_ThrusterControlLoop * tcl;
    float avgDistance;
    float distanceInput;
    float angSpeed;
    float fwdSpeed;
    float corridorWidth;
    float initialHeading;
    float targetHeading;
    float diff;
    QString wallState;
    QString behavState;
    QString adjustState;
    QString guiShow;


signals:
    void timerStart( int msec );
    void timerStop();
    void dataError();
    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void startControlTimer();
    void newWallUiData(const EchoReturnData data, float avgDistance);
    void updateGUI(QString caseW);
    void updateUi();

public slots:
    void updateFromSettings();
    void startBehaviour();
    void stop();
    void reset();
    void terminate();
    void stopOnEchoError();
    void newData(const EchoReturnData data, float avgDistance);
    void controlEnabledChanged(bool);
    void adjustTurnOne();
    void adjustDrive();
    void adjustTurnTwo();
};


#endif // WallFollowingBEHAVIOUR_H
