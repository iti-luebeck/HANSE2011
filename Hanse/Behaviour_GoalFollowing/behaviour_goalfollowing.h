#ifndef GoalBEHAVIOUR_H
#define GoalBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <Behaviour_GoalFollowing/goalfollowingform.h>
#include <Module_VisualSLAM/module_visualslam.h>
#include <Framework/robotmodule.h>

class Module_ThrusterControlLoop;
class GoalFollowingForm;

class Behaviour_GoalFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_GoalFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_VisualSLAM* vsl);

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
    QObject robMod;
    QDateTime last;

    void ctrGoalFollowing();

public slots:
    void newData();
};


#endif // GoalBEHAVIOUR_H
