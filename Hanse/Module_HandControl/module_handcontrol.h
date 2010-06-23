#ifndef MODULE_HANDCONTROL_H
#define MODULE_HANDCONTROL_H

#include <Framework/robotbehaviour.h>

class Server;
class Module_Thruster;
class Module_ThrusterControlLoop;
class HandControl_Form;

class Module_HandControl : public RobotBehaviour {
    Q_OBJECT
    friend class HandControl_Form;

public:
    Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster* thrusterLeft, Module_Thruster* thrusterRight, Module_Thruster* thrusterDown, Module_Thruster* thrusterDownFront);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    void reset();
    void terminate();

    void start();
    void stop();
    bool isActive();

private:

    Module_Thruster* thrusterLeft;
    Module_Thruster* thrusterRight;
    Module_Thruster* thrusterDown;
    Module_Thruster* thrusterDownFront;
    Module_ThrusterControlLoop *controlLoop;

    Server* server;

    void sendNewControls();

private slots:
    void newMessage(int forwardSpeed, int angularSpeed, int speedUpDown);
    void serverReportedError(QString error);

};

#endif // MODULE_HANDCONTROL_H

