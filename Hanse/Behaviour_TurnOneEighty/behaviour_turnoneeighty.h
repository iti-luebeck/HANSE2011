#ifndef BEHAVIOUR_TURNONEEIGHTY_H
#define BEHAVIOUR_TURNONEEIGHTY_H

#include <Framework/robotbehaviour.h>

#define TURN_DEFAULT_P          0.4
#define TURN_DEFAULT_HYSTERESIS 10

class Module_ThrusterControlLoop;
class Module_XsensMTi;

class Behaviour_TurnOneEighty : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_TurnOneEighty( QString id, Module_ThrusterControlLoop* tcl, Module_XsensMTi *x);
    QList<RobotModule*> getDependencies();
    QWidget* createView(QWidget *parent);

    bool isActive();

public slots:
    void startBehaviour();
    void stop();
    void xsensUpdate( RobotModule * );
    void initialHeadingUpdate();
    void terminate();

private:
    Module_ThrusterControlLoop* tcl;
    double initialHeading;
    void init();
    Module_XsensMTi *xsens;

signals:
    void setAngularSpeed(float angularSpeed);

};

#endif // BEHAVIOUR_TURNONEEIGHTY_H
