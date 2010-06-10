#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"

#include <Module_Localization/module_localization.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

Module_Navigation::Module_Navigation(QString id, Module_Localization *localization, Module_ThrusterControlLoop *tcl)
    : RobotModule(id)
{
    this->localization = localization;
    this->tcl = tcl;
}

void Module_Navigation::reset()
{
    RobotModule::reset();
}

void Module_Navigation::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_Navigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(localization);
    ret.append(tcl);
    return ret;
}

QWidget* Module_Navigation::createView(QWidget* parent)
{
    Form_Navigation *form = new Form_Navigation( this, parent );
    QObject::connect( this, SIGNAL( updatedWaypoints(QMap<QString,Position>) ),
                      form, SLOT( updateList(QMap<QString,Position>) ) );
    QObject::connect( form, SIGNAL( removedWaypoint(QString) ),
                      SLOT( removeWaypoint(QString) ) );
    updatedWaypoints( waypoints );
    return form;
}

void Module_Navigation::doHealthCheck()
{

}

void Module_Navigation::addWaypoint( QString name, Position pos )
{
    waypoints[ name ] = pos;
    updatedWaypoints( waypoints );
}

void Module_Navigation::removeWaypoint( QString name )
{
    waypoints.remove( name );
    updatedWaypoints( waypoints );
}
