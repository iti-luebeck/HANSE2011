#include "module_visualslam.h"

#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_VisualSLAM/form_visualslam.h>
#include <QtGui>

Module_VisualSLAM::Module_VisualSLAM( QString id, Module_SonarLocalization *sonarLocalization ) :
        RobotModule(id),
        slam( 100 )
{
    this->sonarLocalization = sonarLocalization;
    cap.setMutex( &updateMutex );
    scene = new QGraphicsScene( QRectF() );

    QObject::connect( &updateTimer, SIGNAL( timeout() ), SLOT( startGrab() ) );
    QObject::connect( &cap, SIGNAL( grabFinished() ), SLOT( startUpdate() ) );
    QObject::connect( &slam, SIGNAL( updateDone() ), SLOT( finishUpdate() ) );

    stopped = true;

    double v_observation = DEFAULT_OBSERVATION_VARIANCE;
    if ( settings.contains( QString("v_observation") ) )
    {
        v_observation = settings["v_observation"];
    }
    double v_translation = DEFAULT_TRANSLATION_VARIANCE;
    if ( settings.contains( QString("v_translation") ) )
    {
        v_translation = settings["v_translation"];
    }
    double v_rotation = DEFAULT_ROTATION_VARIANCE;
    if ( settings.contains( QString("v_rotation") ) )
    {
        v_rotation = settings["v_rotation"];
    }
}

Module_VisualSLAM::~Module_VisualSLAM()
{
    delete( scene );
}

void Module_VisualSLAM::start()
{
    logger->info( "Started" );
    updateTimer.start( 0 );
    stopped = false;
}

void Module_VisualSLAM::stop()
{
    logger->info( "Stopped" );
    updateTimer.stop();
    stopped = true;
}

void Module_VisualSLAM::reset()
{
    RobotModule::reset();
    slam.reset();
}

void Module_VisualSLAM::terminate()
{
    RobotModule::terminate();
}

void Module_VisualSLAM::startGrab()
{
    updateTimer.stop();
    startClock = clock();
    cap.grab();
}

void Module_VisualSLAM::startUpdate()
{
    stopClock = clock();
    logger->debug( QString( "GRAB %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    startClock = clock();
    slam.update( cap.getDescriptors(), cap.getPos(), cap.getClasses() );
}

void Module_VisualSLAM::finishUpdate()
{
    stopClock = clock();
    logger->debug( QString( "UPDATE %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    lastRefreshTime = QDateTime::currentDateTime();

    startClock = clock();
    Position pos = slam.getPosition();
    logger->debug( QString( "POSITION (%1,%2,%3), (%4,%5,%6) with confidence %7" )
                   .arg( pos.getX(), 0, 'f', 3 )
                   .arg( pos.getY(), 0, 'f', 3 )
                   .arg( pos.getZ(), 0, 'f', 3 )
                   .arg( pos.getRoll(), 0, 'f', 3 )
                   .arg( pos.getPitch(), 0, 'f', 3 )
                   .arg( pos.getYaw(), 0, 'f', 3 )
                   .arg( slam.getConfidence(), 0, 'E', 3 ) );

    // Update current position.
    data["Translation - X"] = pos.getX();
    data["Translation - Y"] = pos.getY();
    data["Translation - Z"] = pos.getZ();
    data["Orientation - Yaw"] = pos.getYaw();
    data["Orientation - Pitch"] = pos.getPitch();
    data["Orientation - Roll"] = pos.getRoll();
    data["Confidence"] = slam.getConfidence();

    // Update objects.
    QRectF boundingBox;
    QDateTime lastSeen;
    cap.getObjectPosition( GOAL_LABEL, boundingBox, lastSeen );
    data["Goal - bounding box x"] = boundingBox.left();
    data["Goal - bounding box y"] = boundingBox.top();
    data["Goal - bounding box w"] = boundingBox.width();
    data["Goal - bounding box h"] = boundingBox.height();
    data["Goal - lastSeen"] = lastSeen.toString();

    if ( scene != NULL )
    {
        slam.plot( this->scene );
    }

    stopClock = clock();
    logger->debug( QString( "PLOT %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    emit dataChanged( this );
    emit updateFinished();
    emit viewUpdated( scene );

    if ( !stopped )
    {
        updateTimer.start( 0 );
    }
}

void Module_VisualSLAM::save( QTextStream &ts )
{
    slam.save( ts );
}

void Module_VisualSLAM::load( QTextStream &ts )
{
    slam.load( ts );
}

QList<RobotModule*> Module_VisualSLAM::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonarLocalization);
    return ret;
}

QWidget* Module_VisualSLAM::createView(QWidget* parent)
{
    Form_VisualSLAM *form = new Form_VisualSLAM( this, parent );
    return form;
}

Position Module_VisualSLAM::getLocalization()
{
    return slam.getPosition();
}

QDateTime Module_VisualSLAM::getLastRefreshTime()
{
    return lastRefreshTime;
}

float Module_VisualSLAM::getLocalizationConfidence()
{
    return slam.getConfidence();
}

bool Module_VisualSLAM::isLocalizationLost()
{
    return false;
}

QMutex *Module_VisualSLAM::getSceneMutex()
{
    return sceneMutex;
}

QMutex *Module_VisualSLAM::getUpdateMutex()
{
    return &updateMutex;
}
