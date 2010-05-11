#ifndef MODULE_UID_H
#define MODULE_UID_H

#include <Framework/robotmodule.h>
#include "QtUID.h"

class Module_UID : public RobotModule {
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
