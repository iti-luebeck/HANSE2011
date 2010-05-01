#ifndef MODULE_THRUSTER_H
#define MODULE_THRUSTER_H

#include "Module_Thruster_global.h"
#include "healthstatus.h"
#include "robotmodule.h"

class Module_UID;

class MODULE_THRUSTERSHARED_EXPORT Module_Thruster : public RobotModule {
    Q_OBJECT

public:
    Module_Thruster(QString id, Module_UID *uid);
    ~Module_Thruster();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

    /**
      * Set speed on thruster. Value ranges between -1 and 1.
      *
      * -1 to 0 --> rotate thruster backwards
      * 0 to 1 --> rotate thruster forwards
      *
      * 0 stops the thruster, while -1 and 1 move the thruster at maximum speed
      * in the aforementioned direction.
      */
    void setSpeed(float speed);

protected slots:

    virtual void doHealthCheck();

signals:
    void healthStatusChanged(HealthStatus data);

private:
    Module_UID *uid;

    void initController();
};

#endif // MODULE_THRUSTER_H
