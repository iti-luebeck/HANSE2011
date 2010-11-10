#ifndef MODULE_HANDCONTROL_H
#define MODULE_HANDCONTROL_H

#include <Framework/robotbehaviour_mt.h>

class Server;
class Module_Thruster;
class Module_ThrusterControlLoop;
class HandControl_Form;

class Module_HandControl : public RobotBehaviour_MT {
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

signals:
    void emergencyStop();
    void startHandControl();
    void setForwardSpeed(float forwardSpeed);
    void setAngularSpeed(float angularSpeed);
    void setDepth(float depth);
    void setUpDownSpeed(float upDownSpeed);
    void setLeftSpeed(float leftSpeed);
    void setRightSpeed(float rightSpeed);

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
    void emergencyStopReceived();
    void startHandControlReceived();

};

#endif // MODULE_HANDCONTROL_H

