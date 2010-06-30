#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule( id )
{
    VI.setUseCallback( true );
    leftConnected = false;
    rightConnected = false;
    bottomConnected = false;
    nCams = 0;

    settingsChanged();
}

Module_Webcams::~Module_Webcams()
{
    stopWebcams();
}

void Module_Webcams::stopWebcams()
{
    if ( leftConnected ) VI.stopDevice( leftID );
    if ( rightConnected ) VI.stopDevice( rightID );
    if ( bottomConnected ) VI.stopDevice( bottomID );
}

void Module_Webcams::grabLeft( IplImage *left )
{
    if ( leftConnected )
    {
        mutex.lock();
        assert( left->width == WEBCAM_WIDTH && left->height == WEBCAM_HEIGHT );
        VI.getPixels( leftID, (unsigned char *)left->imageData, true, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabRight( IplImage *right )
{
    if ( rightConnected )
    {
        mutex.lock();
        assert( right->width == WEBCAM_WIDTH && right->height == WEBCAM_HEIGHT );
        VI.getPixels( rightID, (unsigned char *)right->imageData, true, true );
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( IplImage *bottom )
{
    if ( bottomConnected )
    {
        mutex.lock();
        assert( bottom->width == WEBCAM_WIDTH && bottom->height == WEBCAM_HEIGHT );
        VI.getPixels( bottomID, (unsigned char *)bottom->imageData, true, true );
        mutex.unlock();
    }
}

void Module_Webcams::reset()
{
    mutex.lock();
    stopWebcams();

    if ( this->isEnabled() )
    {
        if ( 0 <= leftID && leftID < nCams )
        {
            VI.setIdealFramerate( leftID, 30 );
            leftConnected = VI.setupDevice( leftID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            leftConnected = false;
        }
        if ( 0 <= rightID && rightID < nCams )
        {
            VI.setIdealFramerate( rightID, 30 );
            rightConnected = VI.setupDevice( rightID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            rightConnected = false;
        }
        if ( 0 <= bottomID && bottomID < nCams )
        {
            VI.setIdealFramerate( bottomID, 30 );
            bottomConnected = VI.setupDevice( bottomID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
        else
        {
            bottomConnected = false;
        }
    }
    mutex.unlock();
}

void Module_Webcams::terminate()
{
    stopWebcams();
}

void Module_Webcams::statusChange( bool value )
{

}

void Module_Webcams::settingsChanged()
{
    nCams = VI.listDevices( true );
    leftID = settings.value( "leftID", 0 ).toInt();
    rightID = settings.value( "rightID", 1 ).toInt();
    bottomID = settings.value( "bottomID", 2 ).toInt();

    if ( isEnabled() )
    {
        reset();
    }
}

QWidget *Module_Webcams::createView( QWidget *parent )
{
    Form_Webcams *cams = new Form_Webcams( this, parent );
    return cams;
}

QList<RobotModule *> Module_Webcams::getDependencies()
{
    return QList<RobotModule *>();
}
