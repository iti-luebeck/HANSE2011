#include "module_webcams.h"
#include <Module_Webcams/form_webcams.h>
#include <assert.h>

Module_Webcams::Module_Webcams( QString id ) :
        RobotModule( id )
{
    leftFramerate = 5;
    rightFramerate = 5;
    bottomFramerate = 5;

    leftCap = NULL;
    rightCap = NULL;
    bottomCap = NULL;

    leftID = 1;
    rightID = 0;
    bottomID = 2;
}

Module_Webcams::~Module_Webcams()
{
    stopWebcams();
    cvReleaseImage( &leftFrame );
    cvReleaseImage( &rightFrame );
    cvReleaseImage( &bottomFrame );
}

void Module_Webcams::init()
{
    nCams = this->numAvailableCams();

    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );

    this->setSettingsValue("leftEnabled", false);
    this->setSettingsValue("rightEnabled", false);
    this->setSettingsValue("bottomEnabled", false);

    settingsChanged();
}

void Module_Webcams::stopWebcams()
{
    if ( leftCap != NULL )
    {
        cvReleaseCapture(&leftCap);
        leftCap = NULL;
    }
    if ( rightCap != NULL )
    {
        cvReleaseCapture(&rightCap);
        rightCap = NULL;
    }
    if ( bottomCap != NULL  )
    {
        cvReleaseCapture(&bottomCap);
        bottomCap = NULL;
    }
}

void Module_Webcams::grabLeft( cv::Mat &left )
{
    dataLockerMutex.lock();
    if ( leftCap != NULL )
    {

        IplImage* img = cvQueryFrame(leftCap);
        if(img)
        {
            cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
            cv::Mat mtx(img);

            left = mtx.clone();
        }
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::grabRight( cv::Mat &right )
{
    dataLockerMutex.lock();
    if ( rightCap != NULL )
    {
        IplImage* img = cvQueryFrame(rightCap);
        if(img)
        {
            cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
            cv::Mat mat(img);

            right = mat.clone();
        }
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::grabBottom( cv::Mat &bottom )
{
    dataLockerMutex.lock();
    if ( bottomCap != NULL )
    {
        IplImage* img = cvQueryFrame(bottomCap);
        if(img)
        {
            cvConvertImage(img,img,CV_CVTIMG_SWAP_RB);
            cv::Mat mat(img);
            bottom = mat.clone();
        }
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::grabLeft(IplImage *left)
{
    dataLockerMutex.lock();
    if(leftCap != NULL)
    {
        assert( left->width == WEBCAM_WIDTH && left->height == WEBCAM_HEIGHT );
        cvCopy(cvQueryFrame(leftCap),left);
        cvConvertImage(left,left,CV_CVTIMG_SWAP_RB);
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::grabRight(IplImage *right)
{
    dataLockerMutex.lock();
    if(rightCap != NULL)
    {
        assert( right->width == WEBCAM_WIDTH && right->height == WEBCAM_HEIGHT );
        cvCopy(cvQueryFrame(rightCap),right);
        cvConvertImage(right,right,CV_CVTIMG_SWAP_RB);
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::grabBottom(IplImage *bottom)
{
    dataLockerMutex.lock();
    if(bottomCap != NULL)
    {
        assert( bottom->width == WEBCAM_WIDTH && bottom->height == WEBCAM_HEIGHT );
        cvCopy(cvQueryFrame(bottomCap),bottom);
        cvShowImage("buhu",bottom);
        cvConvertImage(bottom,bottom,CV_CVTIMG_SWAP_RB);
    }
    dataLockerMutex.unlock();
}

void Module_Webcams::reset()
{
    qDebug() << "cams reset";
    dataLockerMutex.lock();
    stopWebcams();
    this->nCams = this->numAvailableCams();
    int connectedCams = 0;

    if ( this->isEnabled() )
    {
        qDebug() << "gonna connect";
        if ( 0 <= bottomID && bottomEnabled && connectedCams < nCams)
        {
            logger->debug("Trying to Connect bottom Cam with ID "+QString::number(bottomID));
            bottomCap = cvCaptureFromCAM(bottomID);
            if(bottomCap)
            {
                logger->debug("...ok");
                connectedCams++;
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_HEIGHT,480);
                cvSetCaptureProperty(bottomCap,CV_CAP_PROP_GAIN,0);
                qDebug() << QString::number(cvGetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_WIDTH));
                qDebug() << QString::number(cvGetCaptureProperty(bottomCap,CV_CAP_PROP_FRAME_HEIGHT));
            }
            else
            {
                logger->debug("...fail");
                bottomCap = NULL;
            }
        }

        if ( 0 <= leftID && leftEnabled && connectedCams < nCams && (leftID != bottomID || !bottomEnabled) )
        {
            logger->debug("Trying to Connect left Cam with ID "+QString::number(leftID));
            leftCap = cvCaptureFromCAM(leftID);
            if (leftCap)
            {
                logger->debug("...ok");
                connectedCams++;
                cvSetCaptureProperty(leftCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(leftCap,CV_CAP_PROP_FRAME_HEIGHT,480);
            }
            else
            {
                logger->debug("...fail");
                leftCap = NULL;
            }

        }
        if ( 0 <= rightID && rightEnabled && connectedCams < nCams && (rightID != leftID || !leftEnabled) && (rightID != bottomID || bottomEnabled))
        {
            logger->debug("Trying to Connect right Cam with ID "+QString::number(rightID));
            rightCap = cvCaptureFromCAM(rightID);
            if(rightCap)
            {
                logger->debug("...ok");
                connectedCams++;
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_FRAME_WIDTH,640);
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_FRAME_HEIGHT,480);
                cvSetCaptureProperty(rightCap,CV_CAP_PROP_GAIN,0);
            }
            else
            {
                logger->debug("...fail");
                rightCap = NULL;
            }
        }
    }
    dataLockerMutex.unlock();
}

std::vector<int> Module_Webcams::numOfCams()
{
    return this->camInd;
}

int Module_Webcams::numAvailableCams()
{
    this->camInd.clear();
    QString path = "/sys/class/video4linux/";
    QDir dir(path);
    dir.setNameFilters(QStringList("video*"));
    QStringList entries = dir.entryList();
    for(int i = 0; i < entries.size(); i++)
    {
        dir.cd(entries.at(i));

        QFile file(dir.path().append("/name"));
        file.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        QString camName = stream.readLine();
        if(camName != "WebCam")
        {
            QString id = entries.at(i);
            this->camInd.push_back(id.remove("video").toInt());
        }
        dir.cdUp();
    }
    return camInd.size();
}

void Module_Webcams::terminate()
{
    stopWebcams();
    RobotModule::terminate();
}

void Module_Webcams::settingsChanged()
{
    leftID = this->getSettingsValue("leftID",0).toInt();
    rightID = this->getSettingsValue("rightID",1).toInt();
    bottomID = this->getSettingsValue("bottomID",2).toInt();
    leftEnabled = this->getSettingsValue("leftEnabled",false).toBool();
    rightEnabled = this->getSettingsValue("rightEnabled",false).toBool();
    bottomEnabled = this->getSettingsValue("bottomEnabled",false).toBool();
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
