#ifndef GoalBEHAVIOUR_H
#define GoalBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_GoalFollowing/goalfollowingform.h>
//#include <Module_VisualSLAM/module_visualslam.h>
#include <Framework/robotmodule.h>

#define STATE_IDLE 100
#define STATE_SEEN_GOAL 101
#define STATE_TURNING 102
#define STATE_FORWARD 103
#define STATE_FAILED  104

class Module_ThrusterControlLoop;
class GoalFollowingForm;

class Behaviour_GoalFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_GoalFollowing(QString id, Module_ThrusterControlLoop* tcl);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    bool isActive();


private:

    Module_ThrusterControlLoop* tcl;
    QDateTime last;
    QTimer timerNoGoal;
    int state;

    void ctrGoalFollowing();

public slots:
    void newData(int classNr);
    void start();
    void stop();
    void reset();

private slots:
    void timerSlot();


};


#endif // GoalBEHAVIOUR_H
