#ifndef MODULE_PRESSURESENSOR_H
#define MODULE_PRESSURESENSOR_H

#include <QtCore>
#include "Module_PressureSensor_global.h"
#include "robotmodule.h"
#include "module_uid.h"
#include "inttypes.h"

class MODULE_PRESSURESENSORSHARED_EXPORT Module_PressureSensor : public RobotModule {
    Q_OBJECT

public:
    Module_PressureSensor(QString id, Module_UID *uid);
    ~Module_PressureSensor();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    void terminate();

    /**
      * Return depth below the surface in meters.
      *
      * Range: -infinity to 0
      *
      */
    float getDepth();

signals:
    void healthStatusChanged(HealthStatus data);

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;

    uint16_t depth;

};

#endif // MODULE_PRESSURESENSOR_H
