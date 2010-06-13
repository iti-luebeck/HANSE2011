#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"

#include <Module_Localization/module_localization.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

Module_Navigation::Module_Navigation(QString id, Module_Localization *localization, Module_ThrusterControlLoop *tcl) :
        RobotModule(id),
        scene( 1, 1, 638, 478, NULL )
{
    this->localization = localization;
    this->tcl = tcl;
    updateTimer.start( 1000 );
    QObject::connect( &updateTimer, SIGNAL( timeout() ), SLOT( plot() ) );
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
    QObject::connect( this, SIGNAL( updatedView(QGraphicsScene *) ),
                      form, SLOT( updateView(QGraphicsScene *) ) );
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

void Module_Navigation::plot()
{
    localization->plot( &scene );
    updatedView( &scene );
}

void Module_Navigation::save( QString path )
{    
    QString slamFile = path;
    slamFile.append( "/slam.txt" );

    QFile file( slamFile );
    file.open( QIODevice::WriteOnly );
    QTextStream ts( &file );
    localization->save( ts );
}
