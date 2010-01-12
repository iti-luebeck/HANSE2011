#ifndef MODULE_SERIALPORT_H
#define MODULE_SERIALPORT_H

#include "Module_SerialPort_global.h"
#include <robotmodule.h>
//#include <log4qt/logger.h>

class MODULE_SERIALPORTSHARED_EXPORT Module_SerialPort : public RobotModule {

    Q_OBJECT
//    LOG4QT_DECLARE_QCLASS_LOGGER

public:
    Module_SerialPort(QString id);

    QString getTabName();

    QWidget* createView(QWidget* parent);

    // TODO: getView();
    QList<RobotModule*> getDependencies();

public slots:
    void reset();
    //void enabled(bool value);

};

#endif // MODULE_SERIALPORT_H
