#include "module_localization.h"
#include "form_localization.h"
#include <QtGui>

#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_PressureSensor/module_pressuresensor.h>

Module_Localization::Module_Localization(QString id, Module_VisualSLAM *visualSLAM, Module_SonarLocalization *sonarLoc, Module_PressureSensor *pressure)
    : RobotModule(id)
{
    this->visualSLAM = visualSLAM;
    this->sonarLocalization = sonarLoc;
    this->pressure = pressure;
    scene = new QGraphicsScene( QRectF() );

    QObject::connect( visualSLAM, SIGNAL( viewUpdated( QGraphicsScene * ) ),
                      SLOT( updateView( QGraphicsScene * ) ) );
}

Module_Localization::~Module_Localization()
{
    delete scene;
}

Position Module_Localization::getLocalization()
{
    return visualSLAM->getLocalization();
}

QDateTime Module_Localization::getLastRefreshTime()
{
    return visualSLAM->getLastRefreshTime();
}

float Module_Localization::getLocalizationConfidence()
{
    return visualSLAM->getLocalizationConfidence();
}

bool Module_Localization::isLocalizationLost()
{
    return visualSLAM->isLocalizationLost();
}

void Module_Localization::save( QTextStream &ts )
{
    visualSLAM->save( ts );
}

void Module_Localization::load( QTextStream &ts )
{
    visualSLAM->load( ts );
}

void Module_Localization::reset()
{
    RobotModule::reset();
}

void Module_Localization::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_Localization::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(visualSLAM);
    ret.append(sonarLocalization);
    return ret;
}

QWidget* Module_Localization::createView(QWidget* parent)
{
    return new Form_Localization(parent);
}

void Module_Localization::doHealthCheck()
{

}

void Module_Localization::updateView( QGraphicsScene *scene )
{
    this->scene->clear();
    QList<QGraphicsItem *> items = scene->items();
    for ( int i = 0; i < items.size(); i++ )
    {
        this->scene->addItem( items[i] );
    }
    emit viewUpdated( this->scene );
}
