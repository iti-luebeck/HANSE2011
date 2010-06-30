#include "module_visualslam.h"

#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_Webcams/module_webcams.h>
#include <Module_VisualSLAM/form_visualslam.h>
#include <QtGui>

Module_VisualSLAM::Module_VisualSLAM( QString id, Module_SonarLocalization *sonarLocalization,
                                      Module_Webcams *cams ) :
        RobotModule(id),
        slam( 50 ),
        cap( cams )
{
    updateThread.start(QThread::LowPriority);
    updateThread.moveTimer();

    this->sonarLocalization = sonarLocalization;
    this->cams = cams;

    QObject::connect( &updateThread.timer, SIGNAL( timeout() ), SLOT( startGrab() ),
                      Qt::DirectConnection );
    QObject::connect( &cap, SIGNAL( grabFinished() ), SLOT( startUpdate() ) );
    QObject::connect( &slam, SIGNAL( updateDone() ), SLOT( finishUpdate() ) );
    QObject::connect( sonarLocalization, SIGNAL(newLocalizationEstimate()),
                      this, SLOT(updateSonarData()) );
    QObject::connect( this, SIGNAL( enabled(bool) ), this, SLOT( statusChange(bool) ) );
    QObject::connect( this, SIGNAL( timerStart(int) ), &updateThread.timer, SLOT( start(int) ) );
    QObject::connect( this, SIGNAL( timerStop() ), &updateThread.timer, SLOT( stop() ),
                      Qt::BlockingQueuedConnection );

    double v_observation = settings.value( "v_observation", DEFAULT_OBSERVATION_VARIANCE ).toDouble();
    double v_translation = settings.value( "v_translation", DEFAULT_TRANSLATION_VARIANCE ).toDouble();
    double v_rotation = settings.value( "v_rotation", DEFAULT_ROTATION_VARIANCE ).toDouble();
    changeSettings( v_observation, v_translation, v_rotation );

    if ( isEnabled() )
    {
        start();
    }

//    QObject::connect( &testTimer, SIGNAL(timeout()), this, SLOT(test()) );
//    testTimer.start(100);
}

Module_VisualSLAM::~Module_VisualSLAM()
{
}

void Module_VisualSLAM::setEnabled( bool value )
{
    RobotModule::setEnabled( value );
}

void Module_VisualSLAM::statusChange( bool value )
{
    if ( value )
    {
        this->start();
    }
    else
    {
        this->stop();
    }
}

void Module_VisualSLAM::reset()
{
    logger->info( "Reset" );
    RobotModule::reset();
    slam.reset();
}

void Module_VisualSLAM::terminate()
{
    logger->info( "Terminated" );
    RobotModule::terminate();
}

void Module_VisualSLAM::start()
{
    logger->info( "Started" );
    cap.init();

    emit timerStart( 1000 );
}

void Module_VisualSLAM::stop()
{
    logger->info( "Stopped" );
    emit timerStop();
}

void Module_VisualSLAM::startGrab()
{
//    qDebug("blub: %d", QThread::currentThread());

    startClock = clock();
    cap.grab( settings.value( QString( "capture" ), false ).toBool() );

    stopClock = clock();
    logger->debug( QString( "GRAB %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    startClock = clock();
    slam.update( cap.getDescriptors(), cap.getPos(), cap.getClasses() );

    stopClock = clock();
    logger->debug( QString( "UPDATE %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    lastRefreshTime = QDateTime::currentDateTime();

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

    emit dataChanged( this );
    emit updateFinished();
    emit viewUpdated();
    emit newLocalizationEstimate();
}

void Module_VisualSLAM::startUpdate()
{
}

void Module_VisualSLAM::finishUpdate()
{
}

void Module_VisualSLAM::updateObject( int classNr )
{
    emit foundNewObject( classNr );
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

void Module_VisualSLAM::getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen )
{
    cap.getObjectPosition( classNr, boundingBox, lastSeen );
}

void Module_VisualSLAM::getPlotData( QList<Position> &landmarkPositions, QList<Position> &particlePositions, int &bestParticle )
{
    bestParticle = slam.getBestParticle();
    slam.getLandmarkPositions( landmarkPositions, bestParticle );
    slam.getParticlePositions( particlePositions );
}

IplImage *Module_VisualSLAM::getFrame( int camNr )
{
    return cap.getFrame( camNr );
}

double Module_VisualSLAM::getObservationVariance()
{
    return settings.value( "v_observation", DEFAULT_OBSERVATION_VARIANCE ).toDouble();
}

double Module_VisualSLAM::getTranslationVariance()
{
    return settings.value( "v_translation", DEFAULT_TRANSLATION_VARIANCE ).toDouble();
}

double Module_VisualSLAM::getRotationVariance()
{
    return settings.value( "v_rotation", DEFAULT_ROTATION_VARIANCE ).toDouble();
}

void Module_VisualSLAM::changeSettings( double v_observation, double v_translation, double v_rotation )
{
    slam.setObservationVariance( v_observation );
    settings.setValue( "v_observation", v_observation );
    slam.setTranslationVariance( v_translation );
    settings.setValue( "v_translation", v_translation );
    slam.setRotationVariance( v_rotation );
    settings.setValue( "v_rotation", v_rotation );
}

void Module_VisualSLAM::updateSonarData()
{
    //TODO welches feld auslesen???
    Position sonarPos = sonarLocalization->getLocalization();
    Position vslamPos = getLocalization();

    // Calculate position difference.
    Position diffPos = sonarPos - vslamPos;
    diffPos.setZ( .0 );
    diffPos.setPitch( .0 );
    diffPos.setRoll( .0 );

    // Set position difference as offset.
    slam.setOffset( diffPos );
}

void Module_VisualSLAM::test()
{
    qDebug("bla: %d", QThread::currentThread());
}
