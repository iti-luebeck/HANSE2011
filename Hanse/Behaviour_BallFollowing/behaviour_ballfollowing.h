#ifndef BallBEHAVIOUR_H
#define BallBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_BallFollowing/ballfollowingform.h>
#include <Module_VisualSLAM/module_visualslam.h>
#include <Framework/robotmodule.h>

class Module_ThrusterControlLoop;
class GoalFollowingForm;

class Behaviour_BallFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_VisualSLAM* vsl);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** starts GoalFollow Behaviour */
    void start();
    /** stops GoalFollow Behaviour */
    void stop();

    void reset();
    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();


private:

    Module_ThrusterControlLoop* tcl;
    Module_VisualSLAM* vsl;
    QDateTime last;
    QTimer timerNoBall;

    void ctrBallFollowing();

public slots:
    void newData(int classNr);
    void timerSlot();
};


#endif // GoalBEHAVIOUR_H
