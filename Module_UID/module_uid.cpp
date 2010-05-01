#include "module_uid.h"

#include <iostream>
#include <form_uid.h>
#include <qextserialenumerator.h>
#include <qextserialport.h>
#include <string>
#include <QtCore/QObject>
#include <QVector>
#include <QString>
#include <QtGlobal>
#include "QtUID.h"

Module_UID::Module_UID(QString moduleId)
    :RobotModule(moduleId)
{
    uid = new UID(settings.value("uidId", DEFAULT_UID_ID).toString());
}

Module_UID::~Module_UID()
{
    delete uid;
}

void Module_UID::reset()
{
}

QList<RobotModule*> Module_UID::getDependencies()
{
    return QList<RobotModule*>();
}

QWidget* Module_UID::createView(QWidget* parent)
{
    return new FormUID(this, parent);
}

void Module_UID::terminate()
{
    //
}

UID* Module_UID::getUID()
{
    return uid;
}
