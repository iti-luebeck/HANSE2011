#ifndef MODULE_SCANNINGSONAR_H
#define MODULE_SCANNINGSONAR_H

#include "Module_ScanningSonar_global.h"
#include "robotmodule.h"
#include <module_serialport.h>

class MODULE_SCANNINGSONARSHARED_EXPORT Module_ScanningSonar : public RobotModule {
    Q_OBJECT

public:
    Module_ScanningSonar(QString id, Module_SerialPort* serialPort);

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    //void enabled(bool value);

private:
    Module_SerialPort* serialPort;

};

#endif // MODULE_SCANNINGSONAR_H
