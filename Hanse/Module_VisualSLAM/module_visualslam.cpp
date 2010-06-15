#include "module_visualslam.h"

#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_VisualSLAM/form_visualslam.h>
#include <QtGui>

Module_VisualSLAM::Module_VisualSLAM(QString id, Module_SonarLocalization *sonarLocalization) :
        RobotModule(id),
        slam( 100 )
{
    this->sonarLocalization = sonarLocalization;
    cap.setMutex( &updateMutex );

    // updateThread.start();
    // updateTimer.moveToThread( this );
    QObject::connect( &updateTimer, SIGNAL( timeout() ), SLOT( update() ) );

    scene = new QGraphicsScene( 5, 5, 470, 310, NULL );

    QObject::connect( &cap, SIGNAL( grabFinished( vector<CvMat *>, vector<CvScalar>, vector<CvScalar>, vector<int> ) ),
                      SLOT( updateMap( vector<CvMat *>, vector<CvScalar>, vector<CvScalar>, vector<int> ) ) );

    stopped = true;
}

void Module_VisualSLAM::run()
{

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

void Module_VisualSLAM::update()
{
    updateTimer.stop();
    startClock = clock();
    cap.grab();
}

void Module_VisualSLAM::updateMap( vector<CvMat *>descriptors, vector<CvScalar>pos2D,
                                   vector<CvScalar>pos3D, vector<int>classesVector )
{
    stopClock = clock();
    logger->debug( QString( "GRAB %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );

    startClock = clock();
    slam.update( descriptors, pos3D, classesVector );
    for ( int i = 0; i < descriptors.size(); i++ )
    {
        cvReleaseMat( &descriptors[i] );
    }
    descriptors.clear();
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
    data["Goal - bounding box x"] = boundingBox.left();;
    data["Goal - bounding box y"] = boundingBox.top();
    data["Goal - bounding box w"] = boundingBox.width();
    data["Goal - bounding box h"] = boundingBox.height();
    data["Goal - lastSeen"] = lastSeen.toString();

    emit dataChanged( this );
    emit updateFinished();

    if ( !stopped )
    {
        updateTimer.start( 0 );
    }
}

void Module_VisualSLAM::plot( QGraphicsScene *scene )
{
    sceneMutex.lock();
    startClock = clock();

//    QImage image1((unsigned char*)cap.getFrame(0)->imageData, 640, 480, QImage::Format_RGB888);
//    image1 = image1.scaledToHeight(120);
//    image1 = image1.scaledToWidth(160);
//
//    QGraphicsPixmapItem *pitem1 = scene->addPixmap( QPixmap::fromImage( image1 ) );
//    pitem1->setPos( 50, 190 );
//
//    QImage image2((unsigned char*)cap.getFrame(1)->imageData, 640, 480, QImage::Format_RGB888);
//    image2 = image2.scaledToHeight(120);
//    image2 = image2.scaledToWidth(160);
//
//    QGraphicsPixmapItem *pitem2 = scene->addPixmap( QPixmap::fromImage( image2 ) );
//    pitem2->setPos( 260, 190 );

    slam.plot( scene );
    stopClock = clock();
    logger->debug( QString( "PLOT %1 msec" ).arg( (1000 * (stopClock - startClock) / CLOCKS_PER_SEC) ) );
    sceneMutex.unlock();
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
    form->setScene( scene );
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
