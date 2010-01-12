#include "module_scanningsonar.h"
#include <QWidget>
#include <QMessageBox>

Module_ScanningSonar::Module_ScanningSonar(QString id, Module_SerialPort* serialPort)
    : RobotModule(id)
{
    this->serialPort = serialPort;
}

void Module_ScanningSonar::reset()
{
    QMessageBox::information(0, "Reset signal on Scanning Sonar", "bla", QMessageBox::Ok);
}

/*
void Module_ScanningSonar::enabled(bool value)
{
    //bool en = settings.value("enabled").toBool();
    m_enabled = value;

    emit isEnabled(m_enabled);
}
*/

QList<RobotModule*> Module_ScanningSonar::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(serialPort);
    return ret;
}

QWidget* Module_ScanningSonar::createView(QWidget* parent)
{
    return new QWidget(parent);
}
