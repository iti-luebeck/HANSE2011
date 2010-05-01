#ifndef MODULE_PRESSURESENSOR_H
#define MODULE_PRESSURESENSOR_H

#include <QtCore>
#include "Module_PressureSensor_global.h"
#include "robotmodule.h"
#include "inttypes.h"

class Module_UID;

class MODULE_PRESSURESENSORSHARED_EXPORT Module_PressureSensor : public RobotModule {
    Q_OBJECT

public:
    Module_PressureSensor(QString id, Module_UID *uid);
    ~Module_PressureSensor();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

    /**
      * Return depth below the surface in meters.
      *
      * Range: 0 to infinity
      *
      */
    float getDepth();

public slots:
    void reset();
    void terminate();


signals:
    void healthStatusChanged(HealthStatus data);

    /**
      * This signal is emmited whenever there a new messurement is available.
      *
      */
    void newDepthData(float depth);

private slots:
    void refreshData();

private:
    Module_UID *uid;
    QTimer timer;

    uint16_t depth;

};

#endif // MODULE_PRESSURESENSOR_H
