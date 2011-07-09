#ifndef BallBEHAVIOUR_H
#define BallBEHAVIOUR_H

#include <Framework/robotbehaviour.h>
#include <Behaviour_BallFollowing/balltracker.h>

#define BALL_STATE_SEARCH_BALL  "search ball"
#define BALL_STATE_FOUND_BALL   "found ball"
#define BALL_STATE_CUT_BALL     "cut ball"
#define BALL_STATE_DO_CUT       "do cut"

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
    BallTracker *getTracker();
    void setState(QString state);

private:
    void update();
    void init();


private:

    bool active;

    Module_ThrusterControlLoop* tcl;
    Module_Webcams* cams;
    Module_XsensMTi *xsens;
    Module_Simulation *sim;

    BallTracker tracker;
    cv::Mat frame;

    QDateTime last;
    QTimer cutTimer;
    QTimer updateTimer;
    QString state;
    float cutHeading;

public slots:
    void testBehaviour( QString path );
    void xsensUpdate( RobotModule * );
    void startBehaviour();
    void stop();
    void reset();
    void terminate();
    void controlEnabledChanged(bool);
    void newFrame(cv::Mat frame);

private slots:
    void newData();
    void stopCut();


signals:
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
    void requestFrame();
    void printFrame(IplImage *image);
    void newBallState(QString state);
};


#endif // GoalBEHAVIOUR_H
