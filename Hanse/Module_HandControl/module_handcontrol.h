#ifndef MODULE_HANDCONTROL_H
#define MODULE_HANDCONTROL_H

#include <Framework/robotmodule.h>

class Server;
class Module_Thruster;
class Module_ThrusterControlLoop;
class HandControl_Form;

class Module_HandControl : public RobotModule {
    Q_OBJECT
    friend class HandControl_Form;

public:
    Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster* thrusterLeft, Module_Thruster* thrusterRight, Module_Thruster* thrusterDown);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    void reset();
    void terminate();

private:

    Module_Thruster* thrusterLeft;
    Module_Thruster* thrusterRight;
    Module_Thruster* thrusterDown;
    Module_ThrusterControlLoop *controlLoop;

    Server* server;

private slots:
    void newMessage(int forwardSpeed, int angularSpeed, int speedUpDown);
    void serverReportedError(QString error);

};

#endif // MODULE_HANDCONTROL_H

