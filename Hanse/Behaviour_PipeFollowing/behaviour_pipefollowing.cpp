#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_PipeFollowing/pipefollowingform.h>
//#include <OpenCV/include/opencv/cv.h>
#include <opencv/cxcore.h>
#include <Module_VisualSLAM/capture/clahe.h>

//using namespace cv;

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));

    setEnabled(false);
    Behaviour_PipeFollowing::noPipeCnt = 0;

    Behaviour_PipeFollowing::firstRun = 1;
 }

bool Behaviour_PipeFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_PipeFollowing::start()
{
    if(!isEnabled())
    {
        Behaviour_PipeFollowing::updateFromSettings();

        if(this->getSettings().value("useCamera").toBool()) this->initCam();
        else vc = VideoCapture(this->getSettings().value("videoFilePath").toString().toStdString());

        logger->debug(this->getSettings().value("videoFilePath").toString());
        logger->debug("cameraID" +QString::number(this->cameraID));
        if((this->connected && this->getSettings().value("useCamera").toBool())
            || (vc.isOpened() && !this->getSettings().value("useCamera").toBool()))
        {
            this->setHealthToOk();
            setEnabled(true);
           timer.start(200);
//            emit started(this);
        }
        else this->setHealthToSick("fail - open camera or video");

    }
}

void Behaviour_PipeFollowing::stop()
{
    if (this->isActive())
    {
       timer.stop();
       vc.release();
       vi.stopDevice(this->cameraID);
       this->tcl->setForwardSpeed(0.0);
       this->tcl->setAngularSpeed(0.0);
       setEnabled(false);
       emit finished(this,false);
   }
}

void Behaviour_PipeFollowing::reset()
{
    RobotBehaviour::reset();
    this->tcl->setForwardSpeed(0.0);
    this->tcl->setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_PipeFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    return ret;
}

QWidget* Behaviour_PipeFollowing::createView(QWidget* parent)
{
    return new PipeFollowingForm( parent, this);
}

void Behaviour_PipeFollowing::timerSlot()
{
    Mat binaryFrame;
    if(this->getSettings().value("useCamera").toBool())
    {
        if(!this->connected) logger->error("cannot retrieve frame from camera");
        else this->grab(frame);
    }
    else
    {
        if(!vc.isOpened())
            logger->error("cannot retrive frame from video file");
        else
            vc >> frame;
    }
    if(!frame.empty())
    {
        this->setHealthToOk();
        Behaviour_PipeFollowing::findPipe(frame,binaryFrame);
        Behaviour_PipeFollowing::computeLineBinary(frame, binaryFrame);
        binaryFrame.release();
        Behaviour_PipeFollowing::updateData();
        Behaviour_PipeFollowing::controlPipeFollow();
    }
    else this->setHealthToSick("empty frame");
}

void Behaviour_PipeFollowing::initCam()
{
    vi.setUseCallback(true);
    vi.listDevices(false);
    vi.setIdealFramerate(this->cameraID,30);
    this->connected = vi.setupDevice(this->cameraID,this->getSettings().value("camWidth").toInt(),this->getSettings().value("camHeight").toInt());

}

void Behaviour_PipeFollowing::grab(Mat &frame)
{
    if (this->connected)
    {
        frame2 = cvCreateImage(cvSize(this->getSettings().value("camWidth").toInt(),this->getSettings().value("camHeight").toInt()), IPL_DEPTH_8U, 3);
        vi.getPixels(this->cameraID, (unsigned char *)frame2->imageData, true, true);
        frame = Mat(frame2,false);
        this->setHealthToOk();
    }
    else this->setHealthToSick("camera not connected");
}


void Behaviour_PipeFollowing::controlPipeFollow()
{
   float ctrAngleSpeed = 0.0;
//   float ctrFwSpeed = 0.0
   float curAbsAngle = (Behaviour_PipeFollowing::curAngle < 0) ?
                       ((-1) * Behaviour_PipeFollowing::curAngle)
                           : Behaviour_PipeFollowing::curAngle;
   float tmp;
   if(curAbsAngle > Behaviour_PipeFollowing::deltaAngPipe)
   {
       ctrAngleSpeed = (-1) * Behaviour_PipeFollowing::kpAngle * Behaviour_PipeFollowing::curAngle / 90.0;
   }

   if(Behaviour_PipeFollowing::distance > Behaviour_PipeFollowing::deltaDistPipe)
   {
       ctrAngleSpeed += Behaviour_PipeFollowing::kpDist * Behaviour_PipeFollowing::distanceY / Behaviour_PipeFollowing::maxDistance;
   }

//   tcl->setAngularSpeed(ctrAngleSpeed);
//   tcl->setForwardSpeed(this->constFWSpeed);
   data["ctrAngleSpeed"] = ctrAngleSpeed;
   data["ctrForwardSpeed"] = this->constFWSpeed;
}

void Behaviour_PipeFollowing::analyzeVideo(QString videoFile)
{
    vc = VideoCapture(videoFile.toStdString());
    if (!vc.isOpened())
    {
        logger->error("cannot open videofile");
    }

   namedWindow("image",1);
    Mat frame, binaryFrame;
    for(;;)
    {
        vc >> frame;
        //            Mat tframe = frame;
        //            transpose(tframe,frame);
        Behaviour_PipeFollowing::findPipe(frame,binaryFrame);
        Behaviour_PipeFollowing::computeLineBinary(frame, binaryFrame);
        waitKey(100);
    }
    vc.release();
}

void Behaviour_PipeFollowing::findPipe(Mat &frame, Mat &binaryFrame)
{
    if (!frame.empty())
    {
        int u;
        Mat frameHSV, h, s, v;
        cvtColor(frame,frameHSV,CV_BGR2HSV);

        /*****convert HSV Image to 3 Binary */
        int rows = frameHSV.rows;
        int cols = frameHSV.cols;
        h.create(rows,cols,CV_8UC1);
        s.create(rows,cols,CV_8UC1);
        v.create(rows,cols,CV_8UC1);
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
                h.at<unsigned char>(i, j) = hsvV[0];
                s.at<unsigned char>(i, j) = hsvV[1];
                v.at<unsigned char>(i, j) = hsvV[2];
            }
        }
        /**** Segmentation */
        Mat thresh;
        IplImage* img = new IplImage(s);
        cvCLAdaptEqualize(img,img,16,16,256,12,CV_CLAHE_RANGE_FULL);
        threshold(s,thresh,threshSegmentation,255,THRESH_BINARY);
        Canny(thresh, binaryFrame, 50, 200, 3 );

        /*debug */
        if(debug)
        {
             namedWindow("Canny",1);
            imshow("Canny",frameHSV);
            u = waitKey();
            imshow("Canny",h);
            u = waitKey();
            imshow("Canny",s);
            u = waitKey();
            imshow("Canny",v);
            u = waitKey();
            imshow("Canny",thresh);
            u = waitKey();
            imshow("Canny",binaryFrame);
            u = waitKey();
            cvDestroyWindow("Canny");
        }
    }
}

void Behaviour_PipeFollowing::computeLineBinary(Mat &frame, Mat &binaryFrame)
{
        /* hough transformation */
        vector<Vec2f> lines;
        HoughLines(binaryFrame, lines, 1, CV_PI/180, 100 );

        /* DEBUG durchschnitt fuer alle erkannten linien */
        float avRho = 0.0;
        float avTheta = 0.0;

        for( size_t i = 0; i < lines.size(); i++ )
        {
            if(debug)
            {
                Behaviour_PipeFollowing::drawLineHough(frame,lines[i][0],lines[i][1],Scalar(0,0,255));
                logger->debug("rho " + QString::number(lines[i][0]));
                logger->debug("theta " + QString::number(lines[i][1]));
            }
            avRho += lines[i][0];
            avTheta += lines[i][1];
        }
        avRho /= lines.size();
        avTheta /= lines.size();

        if(debug)
        {

            logger->debug("****************");
            logger->debug("= " +QString::number(avRho) + " " + QString::number(avTheta));
            Behaviour_PipeFollowing::drawLineHough(frame,avRho,avTheta,Scalar(255,0,255));
        }

        /* jeweils Durchschnitt fuer linke und rechte Seite des Rohrs berechnen */
        float counterClass1 = 0;
        float counterClass2 = 0;
        float avRhoClass1 = 0.0;
        float avRhoClass2 = 0.0;
        float avThetaClass1 = 0.0;
        float avThetaClass2 = 0.0;

        for (size_t i = 0; i < lines.size(); i++)
        {

            if(lines[i][0] < avRho)
            { //Class1
                counterClass1 += 1.0;
                avRhoClass1 += lines[i][0];
                avThetaClass1 += lines[i][1];
            }
            else
            { //Class2
                counterClass2 += 1.0;
                avRhoClass2 += lines[i][0];
                avThetaClass2 += lines[i][1];
            }

        }
        avRhoClass1 /= counterClass1;
        avThetaClass1 /= counterClass1;
        avRhoClass2 /= counterClass2;
        avThetaClass2 /= counterClass2;

        /***** DEBUG linkes und recht berechnen und Zeichnen */
        if(debug)
        {
            logger->debug("Class 1 " + QString::number(avRhoClass1) + " " + QString::number(avThetaClass1));
            logger->debug("Class 2 " + QString::number(avRhoClass2) + " " + QString::number(avThetaClass2));
            Behaviour_PipeFollowing::drawLineHough(frame,avRhoClass1,avThetaClass1,Scalar(255,255,255));
            Behaviour_PipeFollowing::drawLineHough(frame,avRhoClass2,avThetaClass2,Scalar(255,255,255));

        }

        /* Parameter fuers Rohr bestimmen */
        avRhoClass1 = (avRhoClass1 + avRhoClass2) / 2.0;
        avThetaClass1 = (avThetaClass1 + avThetaClass2) / 2.0;
        /* medianfilterung der Werte */
        Behaviour_PipeFollowing::medianFilter(avRhoClass1,avThetaClass1);

        /* ueberpruefen ob rohr zu sehen */
        if(counterClass1 == 0 && counterClass2 == 0)
        {
            this->noPipeCnt++;
            if(noPipeCnt > this->getSettings().value("badFrames").toInt())
            {
                this->setHealthToSick("20 frames without pipe");
                //TODO 180 drehen?
                this->stop();

            }
        }
        else this->noPipeCnt = 0;

        data["rohrRho"] =  avRhoClass1;
        data["rohrTheta"] = avThetaClass1;

        /* Istwinkel */
        curAngle = ((avThetaClass1 * 180.0) / CV_PI);
        if(curAngle > 90)
            curAngle -= 180;

        /* Rohrlinie berechnen und Zeichnen */
        Behaviour_PipeFollowing::drawLineHough(frame,avRhoClass1,avThetaClass1,Scalar(255,255,255));

        /* Schnittpunkt des erkannten Rohrs mit der Ideallinie */
        Behaviour_PipeFollowing::compIntersect(avRhoClass1,avThetaClass1);
        if(debug)
        {
            logger->debug("Schnittpunkt " + QString::number(intersect.x) + " " + QString::number(intersect.y));
            circle(frame,intersect,3,Scalar(255,0,255),3,8);
            circle(frame,robCenter,3,Scalar(255,0,255),3,8);

        }

        /* Potentialvektor berechnen */
        potentialVec = robCenter.y - intersect.y;


        /* Ideallinie zeichnen */
        line( frame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(0,255,0), 3, 8 );

        /* ins Qt Widget malen */
        emit printFrameOnUi(frame);

        if(debug)
        {
            imshow("image",frame);
            waitKey();
        }
}

void Behaviour_PipeFollowing::setThresh(int thresh)
{
    threshSegmentation = thresh;
}

void Behaviour_PipeFollowing::setDebug(int debug)
{
    Behaviour_PipeFollowing::debug = debug;
}

void Behaviour_PipeFollowing::drawLineHough(Mat &frame, double rho, double theta, Scalar color)
{
    double a = cos(theta), b = sin(theta);
    double x1 = a*rho, y1 = b*rho;
    Point pt1(cvRound(x1 + 1000*(-b)),
              cvRound(y1 + 1000*(a)));
    Point pt2(cvRound(x1 - 1000*(-b)),
              cvRound(y1 - 1000*(a)));
    line( frame, pt1, pt2, color, 3, 8 );
}

void Behaviour_PipeFollowing::compIntersect(double rho, double theta)
{
    double a = cos(theta), b = sin(theta);
    double x1 = a*rho, y1 = b*rho;
    Point pt1(cvRound(x1 + 1000.0*(-b)),
              cvRound(y1 + 1000.0*(a)));
    Point pt2(cvRound(x1 - 1000.0*(-b)),
              cvRound(y1 - 1000.0*(a)));
    //    Point richtungsv((pt2.x - pt1.x) , (pt2.y - pt1.y));
    double rv[2] = {(pt2.x - pt1.x) , (pt2.y - pt1.y)};
    float r = ((robCenter.x)/1.0 - pt1.x)/rv[0];
    Point gmp(pt1.x + (((r) * rv[0])) ,
              pt1.y + (((r) * rv[1])));
    intersect = gmp;

    /* Entfernung auf der Bildhalbierenden Y-Achse */
    r = ((robCenter.y)/1.0 - pt1.x)/rv[1];
   distanceY = robCenter.x - (pt1.x + (((r) * rv[0])));

    /*Abstand zwischen ermittelter Gerade und robCenter */
    /*Richtungsvekter double[] rv und Stuetzvektor Point pt1 */
    double zaehler[2] = {((robCenter.x - pt1.x) *rv[0]),((robCenter.y - pt1.y) *rv[1])};
    double nenner[2] = {(rv[0] * rv[0]),(rv[1]*rv[1])};
    double c[2] = {0.0,0.0};
    c[0] = (rv[0] * (zaehler[0] / nenner[0])) + pt1.x;
    c[1] = (rv[1] * (zaehler[1] / nenner[1])) + pt1.y;
    double d[2] ={0, 0};
    d[0] = robCenter.x - c[0];
    d[1] = robCenter.y - c[1];
    distance = sqrt((d[0] * d[0]) + (d[1] * d[1]));

    /* abstand zwischen gmp2 und ideallinie */
    /* ideal zwischen robCenter.x, robCenter.y und robCenter.x und robCenter.y *2 */
    //    double idealRV[2] = {0.0 , robCenter.y};
    //    double normRV = sqrt(idealRV[1] * idealRV[1]);
    //    double ap[2] = {gmp2.x - robCenter.x , gmp2.y - robCenter.y};
    //    double normAP = sqrt((ap[0] * ap[0]) + (ap[1] * ap[1]));
    //    double cosphi = ((((ap[0]) * idealRV[0]) + ((ap[1]) * idealRV[1])) / (normRV * normAP ));
    //    cosphi = acos(cosphi);
    //    cosphi = sin(cosphi);
    //   deltaRohr = normAP * cosphi;
    //    qDebug() << "Abstand " << deltaRohr;
}

void Behaviour_PipeFollowing::setCameraID(int camID)
{
    Behaviour_PipeFollowing::cameraID = camID;
}

void Behaviour_PipeFollowing::setDeltaPipe(float deltaDistPipe, float deltaAngPipe)
{
    Behaviour_PipeFollowing::deltaDistPipe = deltaDistPipe;
    Behaviour_PipeFollowing::deltaAngPipe = deltaAngPipe;
}

void Behaviour_PipeFollowing::setKpDist(float kp)
{
    Behaviour_PipeFollowing::kpDist = kp;
}

void Behaviour_PipeFollowing::setKpAngle(float kp)
{
    Behaviour_PipeFollowing::kpAngle = kp;
}

void Behaviour_PipeFollowing::setRobCenter(double robCenterX, double robCenterY)
{
    Behaviour_PipeFollowing::robCenter = Point(robCenterX, robCenterY);
}

void Behaviour_PipeFollowing::updateData()
{
 data["current Angle"] = this->curAngle;
 data["intersect.x"] = this->intersect.x;
 data["intersect.y"] = this->intersect.y;
 data["distance to robCenter"] = this->distance;
 data["deltaDistPipe"] = this->deltaDistPipe;
 data["deltaAnglePipe"] = this->deltaAngPipe;
 data["kpAngle"] = this->kpAngle;
 data["kpDist"] = this->kpDist;
 data["robCenter.x"] = this->robCenter.x;
 data["robCenter.y"] = this->robCenter.y;
 data["potential Vector"] = this->potentialVec;
}

void Behaviour_PipeFollowing::updateFromSettings()
{
    this->cameraID = this->getSettings().value("cameraID").toInt();
    this->threshSegmentation = this->getSettings().value("threshold").toInt();
    this->debug = this->getSettings().value("debug").toInt();
    this->deltaAngPipe = this->getSettings().value("deltaAngle").toFloat();
    this->deltaDistPipe = this->getSettings().value("deltaDist").toFloat();
    this->kpAngle = this->getSettings().value("kpDist").toFloat();
    this->kpDist = this->getSettings().value("kpAngle").toFloat();
    this->constFWSpeed = this->getSettings().value("fwSpeed").toFloat();
    Behaviour_PipeFollowing::setRobCenter(this->getSettings().value("robCenterX").toDouble(),this->getSettings().value("robCenterY").toDouble());
}

void Behaviour_PipeFollowing::medianFilter(float &rho, float &theta)
{
    logger->debug("in median: rho " +QString::number(rho));
    logger->debug("in median: theta " +QString::number(theta));

    if(std::isnan(rho))
        logger->error("rho is NAN");
    if(std::isnan(theta))
        logger->error("rho is NAN");

    int arrSize = sizeof(Behaviour_PipeFollowing::meanRho) / sizeof(float);
    float sortRho[arrSize];
    float sortTheta[arrSize];

    if(Behaviour_PipeFollowing::firstRun > 0)
    {
        for(int i = 0; i < arrSize;i++)
        {
            Behaviour_PipeFollowing::meanRho[i] = rho;
            Behaviour_PipeFollowing::meanTheta[i] = theta;
        }
        Behaviour_PipeFollowing::firstRun = 0;
    }
    else
    {     
        for(int i = 0; i < arrSize-1; i++)
        {
            Behaviour_PipeFollowing::meanRho[i] = Behaviour_PipeFollowing::meanRho[i+1];
            Behaviour_PipeFollowing::meanTheta[i] = Behaviour_PipeFollowing::meanTheta[i+1];
        }
        Behaviour_PipeFollowing::meanRho[arrSize-1] = rho;
        Behaviour_PipeFollowing::meanTheta[arrSize-1] = theta;

        for(int i = 0; i < arrSize; i++)
        {
            sortRho[i] = Behaviour_PipeFollowing::meanRho[i];
            sortTheta[i] = Behaviour_PipeFollowing::meanTheta[i];
        }

        std::sort(sortRho, sortRho + arrSize);
        std::sort(sortTheta, sortTheta + arrSize);
    }

    rho = sortRho[2];
    theta = sortTheta[2];
}

void Behaviour_PipeFollowing::resetFirstRun()
{
    Behaviour_PipeFollowing::firstRun = 1;
}

