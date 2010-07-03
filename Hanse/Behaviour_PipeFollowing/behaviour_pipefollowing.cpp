#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_PipeFollowing/pipefollowingform.h>
//#include <OpenCV/include/opencv/cv.h>
#include <opencv/cxcore.h>
#include <Module_VisualSLAM/capture/clahe.h>

//using namespace cv;

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Webcams *cam) :
        RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cam = cam;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));

    frame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    displayFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC1 );

    setEnabled(false);
    Behaviour_PipeFollowing::noPipeCnt = 0;

    Behaviour_PipeFollowing::firstRun = 1;
    this->updateFromSettings();
 }

bool Behaviour_PipeFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_PipeFollowing::start()
{
    this->noPipeCnt = 0;
    if(!isEnabled())
    {
        Behaviour_PipeFollowing::updateFromSettings();

//        if(!this->getSettings().value("useCamera").toBool())  vc = VideoCapture(this->getSettings().value("videoFilePath").toString().toStdString());

        logger->debug(this->getSettings().value("videoFilePath").toString());
        logger->debug("cameraID" +QString::number(this->cameraID));
        this->setHealthToOk();
        setEnabled(true);
        timer.start(200);

    }
}

void Behaviour_PipeFollowing::stop()
{
    if (this->isActive())
    {
       timer.stop();
       vc.release();
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
    cam->grabBottom( frame );
    if(!frame.empty())
    {
        this->setHealthToOk();
//        Behaviour_PipeFollowing::findPipe(frame,binaryFrame);
//        Behaviour_PipeFollowing::computeLineBinary(frame, binaryFrame);
        Behaviour_PipeFollowing::moments(frame);
        binaryFrame.release();
        Behaviour_PipeFollowing::updateData();
        Behaviour_PipeFollowing::controlPipeFollow();
    }
    else this->setHealthToSick("empty frame");
}

void Behaviour_PipeFollowing::initPictureFolder()
{
    QDir dir( this->getSettings().value("videoFilePath").toString());
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg";
    dir.setNameFilters( filters );
    files = dir.entryList();
    Mat frame, binaryFrame;
}



void Behaviour_PipeFollowing::controlPipeFollow()
{
   float ctrAngleSpeed = 0.0;
//   float ctrFwSpeed = 0.0
   float tmp;
   if(fabs(Behaviour_PipeFollowing::curAngle) > Behaviour_PipeFollowing::deltaAngPipe)
   {
       ctrAngleSpeed = Behaviour_PipeFollowing::kpAngle * Behaviour_PipeFollowing::curAngle / 90.0;
   }
   float absDistY = Behaviour_PipeFollowing::distanceY;

   if(fabs(Behaviour_PipeFollowing::distanceY) > Behaviour_PipeFollowing::deltaDistPipe)
   {
       ctrAngleSpeed += Behaviour_PipeFollowing::kpDist * Behaviour_PipeFollowing::distanceY / Behaviour_PipeFollowing::maxDistance;
   }

//   tcl->setAngularSpeed(ctrAngleSpeed);
//   tcl->setForwardSpeed(this->constFWSpeed);
   data["ctrAngleSpeed"] = ctrAngleSpeed;
   data["ctrForwardSpeed"] = this->constFWSpeed;
   emit dataChanged( this );
}

void Behaviour_PipeFollowing::analyzeVideo(QString videoFile)
{
    QDir dir( videoFile );
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg";
    dir.setNameFilters( filters );
    files = dir.entryList();
    Mat frame, binaryFrame;

//    namedWindow("Dummy");
    for ( int i = 0; i < files.count(); i++ )
    {
        QString filePath = videoFile;
        filePath.append( "/" );
        filePath.append( files[i] );
        frame = imread( filePath.toStdString() );

        Behaviour_PipeFollowing::moments(frame);
        Behaviour_PipeFollowing::updateData();
        controlPipeFollow();
//        Behaviour_PipeFollowing::findPipe( frame, binaryFrame );
//        Behaviour_PipeFollowing::computeLineBinary( frame, binaryFrame );
        if ( 'q' == waitKey( 500 ) )
            break;
    }

    /*
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
    */
}

void Behaviour_PipeFollowing::findPipe(Mat &frame, Mat &binaryFrame)
{
    if (!frame.empty())
    {
        int u;
        Mat frameHSV;
        cvtColor( frame, displayFrame, CV_RGB2GRAY );
//        cvtColor( frame, frameHSV, CV_RGB2HSV );
//        int channel = settings.value( "channel", 0 ).toInt();
//
//        for (int i = 0; i < WEBCAM_HEIGHT; i++)
//        {
//            for (int j = 0; j < WEBCAM_WIDTH; j++)
//            {
//                Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
//                displayFrame.at<unsigned char>(i, j) = hsvV[channel];
//            }
//        }
        /**** Segmentation */
        Mat threshImg;
//        IplImage *iplDisp = new IplImage( displayFrame );
//        cvCLAdaptEqualize( iplDisp, iplDisp, 8, 8, 255, 12, CV_CLAHE_RANGE_FULL );
        int thresh = settings.value( "threshold", 100 ).toInt();
        threshold( displayFrame, threshImg, thresh, 255, THRESH_BINARY);
//        medianBlur( displayFrame, displayFrame, 5 );
        double width = settings.value( "camWidth", 100 ).toDouble();
        double height = settings.value( "camHeight", 100 ).toDouble();
        Canny( threshImg, binaryFrame, width, height, 3, true );
//        binaryFrame.copyTo( displayFrame );

        /*debug */
        if(debug)
        {
            namedWindow("Canny",1);
            imshow("Canny",frameHSV);
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
        HoughLines(binaryFrame, lines, 1, CV_PI/180, 60 );

        /* DEBUG durchschnitt fuer alle erkannten linien */
        float avRho = 0.0;
        float avTheta = 0.0;

        for( size_t i = 0; i < lines.size(); i++ )
        {
            if ( abs(lines[i][1]) < CV_PI / 12 )
            {
                lines[i][0] = -lines[i][0];
                lines[i][1] = CV_PI + lines[i][1];
            }
        }

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
//            qDebug( "%f, %f", lines[i][0], lines[i][1] );
        }
        avRho /= lines.size();
        avTheta /= lines.size();
        Behaviour_PipeFollowing::drawLineHough( displayFrame, avRho, avTheta,
                                                Scalar(150,0,0) );

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
        if ( counterClass1 > 0 )
        {
            avRhoClass1 /= counterClass1;
            avThetaClass1 /= counterClass1;
        }
        else
        {
            if ( counterClass2 > 0 )
            {
                avRhoClass1 = avRhoClass2 / counterClass2;
                avThetaClass1 = avThetaClass2 / counterClass2;
            }
        }
        if ( counterClass2 > 0 )
        {
            avRhoClass2 /= counterClass2;
            avThetaClass2 /= counterClass2;
        }
        else
        {
            if ( counterClass1 > 0 )
            {
                avRhoClass2 = avRhoClass1 / counterClass1;
                avThetaClass2 = avThetaClass1 / counterClass1;
            }
        }

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

        data["rohrRho"] =  avRho; //avRhoClass1;
        data["rohrTheta"] = avTheta; //avThetaClass1;

        /* Istwinkel */
        curAngle = ((avThetaClass1 * 180.0) / CV_PI);
        if(curAngle > 90)
            curAngle -= 180;

        /* Rohrlinie berechnen und Zeichnen */
        Behaviour_PipeFollowing::drawLineHough( displayFrame, avRhoClass1, avThetaClass1,
                                                Scalar(200,0,0) );

        /* Schnittpunkt des erkannten Rohrs mit der Ideallinie */
        Behaviour_PipeFollowing::compIntersect(avRhoClass1,avThetaClass1);
        if(debug)
        {
            logger->debug("Schnittpunkt " + QString::number(intersect.x) + " " + QString::number(intersect.y));
            circle(frame,intersect,3,Scalar(255,0,255),3,8);
            circle(frame,robCenter,3,Scalar(255,0,255),3,8);

        }



        /* Ideallinie zeichnen */
        line( displayFrame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(255,0,0), 3, 8 );

        /* ins Qt Widget malen */
        emit printFrameOnUi( displayFrame );

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

     potentialVec = robCenter.y - intersect.y;

    /* Entfernung auf der Bildhalbierenden Y-Achse */
    r = ((robCenter.y)/1.0 - pt1.x)/rv[1];
   distanceY = robCenter.x - (pt1.x + (((r) * rv[0])));

    /*Abstand zwischen ermittelter Gerade und robCenter */
    /*Richtungsvekter double[] rv und Stuetzvektor Point pt1 */
//    double zaehler[2] = {((robCenter.x - pt1.x) *rv[0]),((robCenter.y - pt1.y) *rv[1])};
//    double nenner[2] = {(rv[0] * rv[0]),(rv[1]*rv[1])};
//    double c[2] = {0.0,0.0};
//    c[0] = (rv[0] * (zaehler[0] / nenner[0])) + pt1.x;
//    c[1] = (rv[1] * (zaehler[1] / nenner[1])) + pt1.y;
//    double d[2] ={0, 0};
//    d[0] = robCenter.x - c[0];
//    d[1] = robCenter.y - c[1];
//  distance = sqrt((d[0] * d[0]) + (d[1] * d[1]));

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

void Behaviour_PipeFollowing::compIntersect(Point pt1, Point pt2)
{
    //    Point richtungsv((pt2.x - pt1.x) , (pt2.y - pt1.y));
    double rv[2] = {(pt2.x - pt1.x) , (pt2.y - pt1.y)};
    float r = ((robCenter.x)/1.0 - pt1.x)/rv[0];
    Point gmp(pt1.x + (((r) * rv[0])) ,
              pt1.y + (((r) * rv[1])));
    intersect = gmp;

    potentialVec = robCenter.y - intersect.y;


    r = ((robCenter.y)/1.0 - pt1.y)/rv[1];
    potentialY = robCenter.x - (pt1.x + (((r) * rv[0])));
    data["potentialY"] = potentialY;


//    /* Entfernung auf der Bildhalbierenden Y-Achse */
//    r = ((robCenter.y)/1.0 - pt1.x)/rv[1];
//   distanceY = robCenter.x - (pt1.x + (((r) * rv[0])));

    /* ABSTAnd punkt gerade. der richtige */
    double n[2] = {-(pt1.y-pt2.y) , pt1.x - pt2.x};
    double nzero[2] = {(n[0] / sqrt(n[0] * n[0])) , (n[1] / sqrt(n[1] * n[1]))};

    double d = pt1.x * nzero[0] + pt1.y * nzero[1];

//    nzero * p - d
    distanceY = (nzero[0] * robCenter.x) + (nzero[1] * robCenter.y) - d;
    if ( potentialY > 0 )
    {
        distanceY = - fabs( distanceY );
    }
    else
    {
        distanceY = fabs( distanceY );
    }


    data["nullabstand"] = d;
//    distanceY = (pt1.x - robCenter.x) * (pt1.y - pt2.y) + (robCenter.y - pt1.y) * (pt1.x - pt2.x);
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

void Behaviour_PipeFollowing::updateData()
{
 data["current Angle"] = this->curAngle;
 data["intersect.x"] = this->intersect.x;
 data["intersect.y"] = this->intersect.y;
 data["distanceY"] = this->distanceY;
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
    this->cameraID = this->getSettings().value("cameraID",0).toInt();
    this->threshSegmentation = this->getSettings().value("threshold",188).toInt();
    this->debug = this->getSettings().value("debug",0).toInt();
    this->deltaAngPipe = this->getSettings().value("deltaAngle",11).toFloat();
    this->deltaDistPipe = this->getSettings().value("deltaDist",100).toFloat();
    this->kpAngle = this->getSettings().value("kpDist",1).toFloat();
    this->kpDist = this->getSettings().value("kpAngle",1).toFloat();
    this->constFWSpeed = this->getSettings().value("fwSpeed",0.8).toFloat();
    this->robCenter = Point(this->getSettings().value("robCenterX",320).toDouble(),this->getSettings().value("robCenterY",240).toDouble());
    this->maxDistance = this->getSettings().value("maxDistance",320).toFloat();
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

void Behaviour_PipeFollowing::countPixel(Mat &frame, int &sum)
{

    sum = 0;
    for(int i = 0; i < frame.rows; i++)
    {
        for(int j = 0; j < frame.cols; j++)
        {
            if(frame.at<unsigned char>(i,j) > 0)
                sum++;
        }
    }
//    logger->debug("summe" + QString::number(sum));

}

void Behaviour_PipeFollowing::moments( Mat &frame)
{
    Mat binary, gray, frameHSV;
    cvtColor(frame,gray,CV_RGB2GRAY);
    cvtColor(frame,frameHSV,CV_RGB2HSV);
    /*TEST BITTE WIEDER LOESCHEN */
    Mat h, s, v;
    h.create(frame.rows,frame.cols,CV_8UC1);
    s.create(frame.rows,frame.cols,CV_8UC1);
    v.create(frame.rows,frame.cols,CV_8UC1);

    for (int i = 0; i < frame.rows; i++)
    {
        for (int j = 0; j < frame.cols; j++)
        {
            Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
            h.at<unsigned char>(i, j) = hsvV[0];
            s.at<unsigned char>(i, j) = hsvV[1];
            v.at<unsigned char>(i, j) = hsvV[2];

        }
    }
    /*ENDE TEST */

    //    equalizeHist(gray,gray);
    imshow("blub",gray);
    threshold(gray,gray,this->getSettings().value("threshold").toInt(),255,THRESH_BINARY);

    int sum;
    this->countPixel(gray,sum);

    if(sum > 12000)
    {
        noPipeCnt = 0;
        data["pixelSum"] = sum;
//        imshow("Dummy",gray);
        IplImage *ipl = new IplImage(gray);
        CvMoments M;
        cvMoments( ipl, &M, 1 );

        double m00 = cvGetSpatialMoment( &M, 0, 0 );
        double m10 = cvGetSpatialMoment( &M, 1, 0 ) / m00;
        double m01 = cvGetSpatialMoment( &M, 0, 1 ) / m00;
        double mu11 = cvGetCentralMoment( &M, 1, 1 ) / m00;
        double mu20 = cvGetCentralMoment( &M, 2, 0 ) / m00;
        double mu02 = cvGetCentralMoment( &M, 0, 2 ) / m00;
        // moments( binary, m10, m01, mu11, mu02, mu20 );
        double theta = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );
        data["rohrTheta"] = theta;
        if(theta < CV_PI/2)
            theta += CV_PI;
        else if(theta > CV_PI/2)
            theta -= CV_PI;

        if(theta > CV_PI/2)
            theta -= CV_PI;
        else if(theta < -CV_PI/2)
            theta += CV_PI;



        Point pt1 = Point(m10,m01);
        Point pt2 = Point(m10 + cos(theta)*200, m01 + sin(theta)*200);
        /* Ideallinie zeichnen */
        line( frame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(255,0,0), 3, 8 );
        line(frame,Point(m10,m01),Point(m10 + cos(theta)*200, m01 + sin(theta)*200),Scalar(255,0,0),4,CV_FILLED);
        circle(frame,robCenter,3,Scalar(255,0,255),3,8);

        curAngle = ((theta * 180.0) / CV_PI);
        if(curAngle > 90)
            curAngle -= 180;

        if(curAngle > 0)
        {
            curAngle -= 90;
        }
        else if(curAngle < 0)
        {
            curAngle +=90;
        }

        theta = (curAngle * CV_PI) / 180.0;
        data["rohrThetaMod"] = theta;



        //    imshow("image",frame);

        Behaviour_PipeFollowing::compIntersect(pt1,pt2);

        Behaviour_PipeFollowing::updateData();
        emit printFrameOnUi(frame);
    }
    else
    {
        noPipeCnt++;
        tcl->setForwardSpeed(this->getSettings().value("fwSpeed").toFloat()/2);
        if(noPipeCnt > this->getSettings().value("badFrames").toInt())
        {
            this->setHealthToSick("no pipe");
            this->stop();
        }
        emit printFrameOnUi(frame);
    }
}


