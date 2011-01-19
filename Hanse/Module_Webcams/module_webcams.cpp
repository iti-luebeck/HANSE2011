#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule_MT( id )
{
//    VI.setUseCallback( true );
//    leftConnected = false;
//    rightConnected = false;
//    bottomConnected = false;
    leftFramerate = 5;
    rightFramerate = 5;
    bottomFramerate = 5;

    leftCap = NULL;
    rightCap = NULL;
    bottomCap = NULL;


    nCams = this->numOfCams();

    leftID = 1;
    rightID = 0;
    bottomID = 2;

    settingsChanged();



    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
}

Module_Webcams::~Module_Webcams()
{
    stopWebcams();
    cvReleaseImage( &leftFrame );
    cvReleaseImage( &rightFrame );
    cvReleaseImage( &bottomFrame );
}

void Module_Webcams::stopWebcams()
{
    if ( leftCap != NULL )
    {
        delete leftCap;
        leftCap = NULL;
    }
    if ( rightCap != NULL )
    {
        delete rightCap;
        rightCap = NULL;
    }
    if ( bottomCap != NULL  )
    {
        delete bottomCap;
        bottomCap = NULL;
    }

    //cvReleaseCapture(&bottomCam);//VI.stopDevice( bottomID );
}

//void Module_Webcams::grabLeft( IplImage *left )
//{
//    if ( leftConnected )//&& VI.isFrameNew(leftID) )
//    {
//        mutex.lock();
//        assert( left->width == WEBCAM_WIDTH && left->height == WEBCAM_HEIGHT );
//        if(leftCap.grab())
//        {
//            cv::Mat mat;
//            leftCap.retrieve(mat,0);
//            cv::imshow("blub",mat);
//            left = new IplImage(mat.clone());
//        }

////        cvGrabFrame(leftCam);
////        left = cvCloneImage(cvRetrieveFrame(leftCam));
////        VI.getPixels( leftID, (unsigned char *)left->imageData, false, true );
//        mutex.unlock();
//    }
//}

//void Module_Webcams::grabRight( IplImage *right )
//{
//    if ( rightConnected )//&& VI.isFrameNew(rightID))
//    {
//        mutex.lock();
//        assert( right->width == WEBCAM_WIDTH && right->height == WEBCAM_HEIGHT );
//        if(rightCap.grab())
//        {
//            cv::Mat mat;
//            rightCap.retrieve(mat);
//            right = new IplImage(mat);
//        }

////        cvGrabFrame(rightCam);
////        right = cvCloneImage(cvRetrieveFrame(rightCam));
////        VI.getPixels( rightID, (unsigned char *)right->imageData, false, true );
//        mutex.unlock();
//    }
//}

//void Module_Webcams::grabBottom( IplImage *bottom )
//{
//    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
//    {
//        mutex.lock();
//        assert( bottom->width == WEBCAM_WIDTH && bottom->height == WEBCAM_HEIGHT );
//        if(bottomCap.grab())
//        {
//            cv::Mat mat;
//            bottomCap.retrieve(mat);
//            bottom = new IplImage(mat);
//        }
//        //        cvGrabFrame(bottomCam);
////        bottom = cvCloneImage(cvRetrieveFrame(bottomCam));
////        VI.getPixels( bottomID, (unsigned char *)bottom->imageData, false, true );
//        mutex.unlock();
//    }
//}

void Module_Webcams::grabLeft( cv::Mat &left )
{
    if ( leftCap != NULL )
    {
        mutex.lock();
        IplImage* img = cvQueryFrame(leftCap);
        if(img)
        {
        cv::Mat mtx(img);
        left = mtx.clone();
        }
//        *leftCap >> left;
        mutex.unlock();
    }
}

void Module_Webcams::grabRight( cv::Mat &right )
{
    if ( rightCap != NULL )
    {
        mutex.lock();
        IplImage* img = cvQueryFrame(rightCap);
        if(img)
        {
        cv::Mat mat(img);
        right = mat.clone();
    }
//        *rightCap >> right;
        mutex.unlock();
    }
}

void Module_Webcams::grabBottom( cv::Mat &bottom )
{
//    qDebug() << "cam thread id";
//    qDebug() << QThread::currentThreadId();
    if ( bottomCap != NULL )//&& VI.isFrameNew(bottomID))
    {
        mutex.lock();
        IplImage* img = cvQueryFrame(bottomCap);
        if(img)
        {
        cv::Mat mat(img);
        bottom = mat.clone();
        }
//        assert( bottom.cols == WEBCAM_WIDTH && bottom.rows == WEBCAM_HEIGHT );
//        if(bottomCap.grab())
//            *bottomCap >> bottom;
//            bottomCap.retrieve(bottom);

//        cvGrabFrame(bottomCam);
//        IplImage *img = (cvCloneImage(cvRetrieveFrame(bottomCam)));
//        cv::Mat mtx (img);
//        bottom = mtx.clone();
//        VI.getPixels( bottomID, (unsigned char *)bottom.data, true, true );
        mutex.unlock();
    }
}
//
//void Module_Webcams::grabBottom(QImage bottom1)
//{
//    cv::Mat bottom;
//    bottom.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
//    qDebug() << "cam thread id";
//    qDebug() << QThread::currentThreadId();
//    if ( bottomConnected )//&& VI.isFrameNew(bottomID))
//    {
//        mutex.lock();
//        assert( bottom.cols == WEBCAM_WIDTH && bottom.rows == WEBCAM_HEIGHT );
//        VI.getPixels( bottomID, (unsigned char *)bottom.data, true, true );
//        mutex.unlock();
//    }
//     QImage image1((unsigned char*)bottom.data, bottom.cols, bottom.rows, QImage::Format_RGB888);
//     bottom1 = image1.copy();
//}

void Module_Webcams::reset()
{
    mutex.lock();
    stopWebcams();

    if ( this->isEnabled() )
    {
        if ( 0 <= bottomID && bottomID < nCams && bottomEnabled )
        {
            bottomCap = cvCaptureFromCAM(bottomID);
            if(bottomCap)
            {
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_HEIGHT,480);
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_GAIN,0);
                qDebug() << QString::number(cvGetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_WIDTH));
                qDebug() << QString::number(cvGetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_HEIGHT));
            }
            else
            {
                delete bottomCap;
                bottomCap = NULL;
            }
//            bottomCam = cvCaptureFromCAM(bottomID);
//            bottomConnected = true;
//            VI.setIdealFramerate( bottomID, bottomFramerate );
//            bottomConnected = VI.setupDevice( bottomID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }

        if ( 0 <= leftID && leftID < nCams && leftEnabled )
        {
            leftCap = cvCaptureFromCAM(leftID);
            if(leftCap)
            {
                cvSetCaptureProperty(leftCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(leftCap,CV_CAP_PROP_FRAME_HEIGHT,480);
                cvSetCaptureProperty(leftCap,CV_CAP_PROP_GAIN,0);
//                qDebug() << QString::number(leftCap->get(CV_CAP_PROP_FRAME_HEIGHT));
//                qDebug() << QString::number(leftCap->get(CV_CAP_PROP_FRAME_WIDTH));

            }
            else
            {
                delete leftCap;
                leftCap = NULL;
            }

        }
        if ( 0 <= rightID && rightID < nCams && rightEnabled )
        {
//            rightCap = new cv::VideoCapture(3);
            rightCap = cvCaptureFromCAM(rightID);
            if(rightCap)
            {
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_FRAME_HEIGHT,480);
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_GAIN,0);

//                qDebug() << QString::number(rightCap->get(CV_CAP_PROP_FRAME_HEIGHT));
//                qDebug() << QString::number(rightCap->get(CV_CAP_PROP_FRAME_WIDTH));
            }
            else
            {
                delete rightCap;
                rightCap = NULL;
            }
//            VI.setIdealFramerate( rightID, rightFramerate );
//            rightConnected = VI.setupDevice( rightID, WEBCAM_WIDTH, WEBCAM_HEIGHT );
        }
    }
    mutex.unlock();
    qDebug() << "left" << leftConnected;
        qDebug() << "right" << rightConnected;
            qDebug() << "bottom" << bottomConnected;
}

int Module_Webcams::numOfCams()
{ 
    cv::VideoCapture cap;
    int finished = 0;
    int cams = 0;
    return 3;
//    while (!finished)
//    {
//        cap.open(cams);
//        if(!cap.isOpened())
//        {
//            finished = 1;
//        }
//        else
//        {
//            cams++;
//            cap.release();
//        }
//    }
//    return cams;
}

void Module_Webcams::terminate()
{
    stopWebcams();
    RobotModule_MT::terminate();
}




void Module_Webcams::statusChange( bool value )
{

}

void Module_Webcams::showSettings( int camNr )
{
//    VI.showSettingsWindow( camNr );
}

void Module_Webcams::settingsChanged()
{
//    nCams = VI.listDevices( true );
//    nCams = this->numOfCams();
//    logger->debug("Number of Cams "+QString::number(nCams));
    leftID = this->getSettingsValue("leftID",0).toInt();
    rightID = this->getSettingsValue("rightID",1).toInt();
    bottomID = this->getSettingsValue("bottomID",2).toInt();
    leftEnabled = this->getSettingsValue("leftEnabled",true).toBool();
    rightEnabled = this->getSettingsValue("rightEnabled",true).toBool();
    bottomEnabled = this->getSettingsValue("bottomEnabled",true).toBool();
    leftFramerate = this->getSettingsValue("leftFramerate",5).toInt();
    rightFramerate = this->getSettingsValue("rightFramerate",5).toInt();
    bottomFramerate = this->getSettingsValue("bottomFramerate",5).toInt();

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
