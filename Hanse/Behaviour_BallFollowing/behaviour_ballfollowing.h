#ifndef BallBEHAVIOUR_H
#define BallBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <Module_Webcams/module_webcams.h>
#include <Framework/robotmodule.h>

#define BALL_STATE_TURN_45       100
#define BALL_STATE_TRACK_BALL    101
#define BALL_STATE_IDLE          102

class Module_ThrusterControlLoop;
class GoalFollowingForm;
class Module_XsensMTi;

class Behaviour_BallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Webcams* cams, Module_XsensMTi *xsens);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();


private:

    Module_ThrusterControlLoop* tcl;
    Module_Webcams* cams;
    Module_XsensMTi *xsens;
    QDateTime last;
    QTimer timerNoBall;
    QTimer updateTimer;
    int state;
    double targetHeading;

    void ctrBallFollowing();
    void init();

public slots:
    void testBehaviour( QString path );
    void xsensUpdate( RobotModule * );
    void startBehaviour();
    void stop();
    void reset();
    void terminate();

private slots:
    void newData();
    void timerSlot();


signals:
    void printFrame(IplImage *frame);
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
};


#endif // GoalBEHAVIOUR_H
