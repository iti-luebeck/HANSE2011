#include "module_visualslam.h"

#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <QtGui>

Module_VisualSLAM::Module_VisualSLAM(QString id, Module_SonarLocalization *sonarLocalization)
    : RobotModule(id)
{
    this->sonarLocalization = sonarLocalization;

    form = new Form_VisualSLAM( this, NULL );

    // updateThread.start();
    // updateTimer.moveToThread( &updateThread );
    QObject::connect( &updateTimer, SIGNAL( timeout() ), SLOT( update() ) );

    scene = new QGraphicsScene( 5, 5, 470, 310, form );
    form->setScene( scene );

    QObject::connect( &cap, SIGNAL( done() ), SLOT( updateMap() ) );

    stopped = true;
}

void Module_VisualSLAM::start()
{
    logger->info( "Started" );
    updateTimer.start( 700 );
}

void Module_VisualSLAM::stop()
{
    logger->info( "Stopped" );
    updateTimer.stop();
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

void Module_VisualSLAM::update()
{
    updateMutex.lock();
    startClock = clock();
    cap.grab( &des, &pos2D, &pos3D, &classLabels );
}

void Module_VisualSLAM::updateMap()
{
    stopClock = clock();
    logger->debug( QString( "GRAB %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    startClock = clock();
    slam.update( des, pos3D, pos2D, classLabels );
    for ( int i = 0; i < des.size(); i++ )
    {
        cvReleaseMat( &des[i] );
    }
    des.clear();
    stopClock = clock();
    logger->debug( QString( "UPDATE %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );


    sceneMutex.lock();
    startClock = clock();
    scene->clear();

    QImage image1((unsigned char*)cap.getFrame(0)->imageData, 640, 480, QImage::Format_RGB888);
    image1 = image1.scaledToHeight(120);
    image1 = image1.scaledToWidth(160);

    QGraphicsPixmapItem *pitem1 = scene->addPixmap( QPixmap::fromImage( image1 ) );
    pitem1->setPos( 50, 190 );

    QImage image2((unsigned char*)cap.getFrame(1)->imageData, 640, 480, QImage::Format_RGB888);
    image2 = image2.scaledToHeight(120);
    image2 = image2.scaledToWidth(160);

    QGraphicsPixmapItem *pitem2 = scene->addPixmap( QPixmap::fromImage( image2 ) );
    pitem2->setPos( 260, 190 );

    slam.plot( scene );
    stopClock = clock();
    logger->debug( QString( "PLOT %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );
    sceneMutex.unlock();

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

    updateMutex.unlock();

    // Update current position.
    QVector3D translation( pos.getX(), pos.getY(), pos.getZ() );
    data["Translation"] = translation;
    QVector3D orientation( pos.getYaw(), pos.getPitch(), pos.getRoll() );
    data["Orientation"] = orientation;
    data["Confidence"] = slam.getConfidence();

    // Update objects.
    QRectF boundingBox;
    QDateTime lastSeen;
    cap.getObjectPosition( GOAL_LABEL, boundingBox, lastSeen );
    data["Goal - bounding box"] = boundingBox;
    data["Goal - lastSeen"] = lastSeen;

    emit dataChanged( this );
    emit updateFinished();
}

QList<RobotModule*> Module_VisualSLAM::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonarLocalization);
    return ret;
}

QWidget* Module_VisualSLAM::createView(QWidget* parent)
{
    form->setParent( parent );
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
    return &sceneMutex;
}

QMutex *Module_VisualSLAM::getUpdateMutex()
{
    return &updateMutex;
}
