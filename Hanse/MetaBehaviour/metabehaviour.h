#ifndef METABEHAVIOUR_H
#define METABEHAVIOUR_H

#include <Framework/robotmodule.h>
#include <Framework/robotbehaviour.h>

class Module_ThrusterControlLoop;
class Behaviour_PipeFollowing;
class Module_HandControl;
class ModulesGraph;

class MetaBehaviourForm;

class MetaBehaviour : public RobotModule
{
    friend class MetaBehaviourForm;
    Q_OBJECT
public:
    MetaBehaviour(QString id, ModulesGraph* graph, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl);

    QList<RobotModule*> getDependencies();

    void reset();

    QWidget* createView(QWidget *parent);


private:
    Module_ThrusterControlLoop* tcl;
    Module_HandControl* handControl;
    QList<RobotBehaviour*> behaviours;

private slots:
    void emergencyStop();
    void startHandControl();
};


#endif // METABEHAVIOUR_H
