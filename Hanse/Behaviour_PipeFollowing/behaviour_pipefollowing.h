#ifndef PIPEBEHAVIOUR_H
#define PIPEBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

class Module_ThrusterControlLoop;

class Behaviour_PipeFollowing : public RobotBehaviour
{
public:
    Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop* tcl);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    void start();

    void stop();

    bool isActive();

private:

    Module_ThrusterControlLoop* tcl;
};

#endif // PIPEBEHAVIOUR_H
