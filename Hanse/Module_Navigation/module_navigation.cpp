#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_VisualSLAM/module_visualslam.h>

Module_Navigation::Module_Navigation(QString id, Module_SonarLocalization *sonarLoc, Module_VisualSLAM* visSLAM, Module_ThrusterControlLoop *tcl) :
        RobotModule(id)
{
    this->sonarLoc = sonarLoc;
    this->visSLAM = visSLAM;
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
    ret.append( sonarLoc );
    ret.append( visSLAM );
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

void Module_Navigation::save( QString path )
{
    QString slamFile = path;
    slamFile.append( "/slam.txt" );

    QFile file( slamFile );
    file.open( QIODevice::WriteOnly );
    QTextStream ts( &file );
    visSLAM->save( ts );
//    localization->save( ts );
}

void Module_Navigation::load( QString path )
{
    QString slamFile = path;
    slamFile.append( "/slam.txt" );

    QFile file( slamFile );
    file.open( QIODevice::ReadOnly );
    QTextStream ts( &file );
    visSLAM->load( ts );
//    localization->load( ts );
}
