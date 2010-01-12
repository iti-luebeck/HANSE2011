#include "module_serialport.h"
#include "ui_module_serialport_widget.h"
#include <QWidget>
#include <QMessageBox>
#include <qextserialenumerator.h>
#include <stdio.h>

Module_SerialPort::Module_SerialPort(QString id)
    : RobotModule(id)
{
    //logger()->warn("Ahrg!!");
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    printf("List of ports:\n");
    for (int i = 0; i < ports.size(); i++) {
            printf("port name: %s\n", ports.at(i).portName.toLocal8Bit().constData());
            printf("friendly name: %s\n", ports.at(i).friendName.toLocal8Bit().constData());
            printf("physical name: %s\n", ports.at(i).physName.toLocal8Bit().constData());
            printf("enumerator name: %s\n", ports.at(i).enumName.toLocal8Bit().constData());
            printf("===================================\n\n");
    }
}

void Module_SerialPort::reset()
{
    QMessageBox::information(0, "Reset signal on Serial Port", "bla", QMessageBox::Ok);
}

/*
void Module_SerialPort::enabled(bool value)
{
    m_enabled = value;

    emit isEnabled(m_enabled);
}
*/

QList<RobotModule*> Module_SerialPort::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_SerialPort::createView(QWidget* parent)
{
    Ui_module_serialport_widget ui;
    QWidget* w = new QWidget(parent);
    ui.setupUi(w);
    return w;
}

QString Module_SerialPort::getTabName()
{
    return settings.value("port", "COM1").toString();
}
