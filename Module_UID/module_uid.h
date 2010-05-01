#ifndef MODULE_UID_H
#define MODULE_UID_H

#include "robotmodule.h"
#include "Module_UID_global.h"
#include "QtUID.h"

#define DEFAULT_UID_ID "UID0001"

class MODULE_UIDSHARED_EXPORT Module_UID : public RobotModule {
    Q_OBJECT
public:

    Module_UID(QString moduleId);
    Module_UID(QString moduleId, QString deviceId);
    ~Module_UID();

    // inherited from RobotModule
    QWidget* createView(QWidget* parent);
    // inherited from RobotModule
    QList<RobotModule*> getDependencies();

    UID* getUID();

public slots:
    // inherited from RobotModule
    void reset();
    // inherited from RobotModule
    void terminate();

private:
    UID *uid;

};

#endif // MODULE_UID_H
