#ifndef BEHAVIOUR_TURNONEEIGHTY_H
#define BEHAVIOUR_TURNONEEIGHTY_H

#include <Framework/robotbehaviour.h>

#define TURN_DEFAULT_P          0.4
#define TURN_DEFAULT_HYSTERESIS 10

class Module_ThrusterControlLoop;
class Module_Compass;

class Behaviour_TurnOneEighty : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_TurnOneEighty( QString id, Module_ThrusterControlLoop* tcl, Module_Compass *compass );
    QList<RobotModule*> getDependencies();
    QWidget* createView(QWidget *parent);

    bool isActive();

public slots:
    void start();
    void stop();
    void compassUpdate( RobotModule * );
    void initialHeadingUpdate();
    void terminate();

private:
    Module_ThrusterControlLoop* tcl;
    Module_Compass *compass;
    double initialHeading;

signals:
    void setAngularSpeed(float angularSpeed);

};

#endif // BEHAVIOUR_TURNONEEIGHTY_H
