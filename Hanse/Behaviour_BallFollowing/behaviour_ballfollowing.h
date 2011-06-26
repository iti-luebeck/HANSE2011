#ifndef BallBEHAVIOUR_H
#define BallBEHAVIOUR_H

#include <Framework/robotbehaviour.h>
#include <Behaviour_BallFollowing/balltracker.h>

#define BALL_STATE_TURN_45       100
#define BALL_STATE_TRACK_BALL    101
#define BALL_STATE_IDLE          102

class Module_ThrusterControlLoop;
class Module_Webcams;
class GoalFollowingForm;
class Module_XsensMTi;
class Module_Simulation;

class Behaviour_BallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Webcams* cams, Module_XsensMTi *xsens, Module_Simulation *sim);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();
    void grabFrame(cv::Mat &frame);

private:
    void update();


private:
    bool active;

    Module_ThrusterControlLoop* tcl;
    Module_Webcams* cams;
    Module_XsensMTi *xsens;
    Module_Simulation *sim;

    BallTracker tracker;
    cv::Mat frame;

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
    void controlEnabledChanged(bool);
    void simFrame(cv::Mat simFrame);

private slots:
    void newData();
    void timerSlot();


signals:
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
    void requestFrame();
    void printFrame(IplImage *image);
    void newBallState(QString state);
};


#endif // GoalBEHAVIOUR_H
