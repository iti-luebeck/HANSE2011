#ifndef METABEHAVIOUR_H
#define METABEHAVIOUR_H

#include <Framework/robotmodule.h>
#include <Framework/robotbehaviour.h>

class Module_ThrusterControlLoop;
class Behaviour_PipeFollowing;
class Module_HandControl;
class ModulesGraph;
class Module_PressureSensor;
class Behaviour_PipeFollowing;
class MetaBehaviourForm;

class MetaBehaviour : public RobotModule
{
    friend class MetaBehaviourForm;
    Q_OBJECT
public:
    MetaBehaviour(QString id, ModulesGraph* graph, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Behaviour_PipeFollowing* pipe);

    QList<RobotModule*> getDependencies();

    void reset();

    QWidget* createView(QWidget *parent);


private:
    Module_ThrusterControlLoop* tcl;
    Module_HandControl* handControl;
    Module_PressureSensor* pressure;
    Behaviour_PipeFollowing* pipe;
    QList<RobotBehaviour*> behaviours;
    QTimer depthWaitTimer;
    QTimer timeoutTimer;

private slots:
    void emergencyStop();
    void startHandControl();
    void finishedPipe(RobotBehaviour*,bool);
    void depthChanged(float);
    void stateTimeout();

    void badHealth(RobotModule* m);
};


#endif // METABEHAVIOUR_H
