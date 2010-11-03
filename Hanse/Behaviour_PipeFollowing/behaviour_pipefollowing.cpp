#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_PipeFollowing/pipefollowingform.h>
//#include <OpenCV/include/opencv/cv.h>
#include <opencv/cxcore.h>
#include <Module_VisualSLAM/capture/clahe.h>

//using namespace cv;

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Webcams *cam) :
        RobotBehaviour_MT(id)
{
    qDebug() << "pipe thread id";
    qDebug() << QThread::currentThreadId();
    this->tcl = tcl;
    this->cam = cam;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    connect (this,SIGNAL(timerStart(int)),&timer,SLOT(start(int)));
    connect(this, SIGNAL(timerStop()),&timer,SLOT(stop()));

    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    frame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    displayFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC1 );

    setEnabled(false);
    Behaviour_PipeFollowing::noPipeCnt = 0;

    this->updateFromSettings();
//    this->getId();
 }

bool Behaviour_PipeFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_PipeFollowing::start()
{
    if( !isEnabled() )
    {
        this->reset();
        logger->info("Behaviour started" );
        Behaviour_PipeFollowing::updateFromSettings();
        this->setHealthToOk();
        setEnabled(true);
//        timer.start(250);
        emit timerStart(timerTime);

    }
}

void Behaviour_PipeFollowing::stop()
{
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
//    this->tcl->setForwardSpeed(0.0);
//    this->tcl->setAngularSpeed(0.0);
    if (this->isActive())
    {
//        timer.stop();
        emit timerStop();
        logger->info( "Behaviour stopped" );
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
//        this->tcl->setForwardSpeed(0.0);
//        this->tcl->setAngularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
   }
}

void Behaviour_PipeFollowing::terminate()
{
//    timer.stop();
    emit timerStop();
    RobotBehaviour_MT::terminate();
}

void Behaviour_PipeFollowing::reset()
{
    this->curAngle = 0.0;
    this->distanceY = 0.0;
    this->noPipeCnt = 0;
//    RobotBehaviour_MT::reset();
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
//    this->tcl->setForwardSpeed(0.0);
//    this->tcl->setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_PipeFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(cam);
    return ret;
}

QWidget* Behaviour_PipeFollowing::createView(QWidget* parent)
{
    return new PipeFollowingForm( parent, this);
}

void Behaviour_PipeFollowing::timerSlot()
{
    qDebug() << "pipe thread id";
    qDebug() << QThread::currentThreadId();
    QTime run;
    QTime blub;
    run.restart();
//    Mat binaryFrame;
    blub.restart();
    cam->grabBottom( frame );
    addData("run grab",blub.elapsed());

    if(!frame.empty())
    {
        this->setHealthToOk();
//        Behaviour_PipeFollowing::findPipe(frame,binaryFrame);
//        Behaviour_PipeFollowing::computeLineBinary(frame, binaryFrame);
        Behaviour_PipeFollowing::moments(frame);
//        binaryFrame.release();
//        Behaviour_PipeFollowing::updateData();
        if(isEnabled() && this->getHealthStatus().isHealthOk())
        Behaviour_PipeFollowing::controlPipeFollow();
    }
    else this->setHealthToSick("empty frame");

    addData("run",run.elapsed());
    if(getDataValue("run").toInt() > timerTime)
    {
        this->setHealthToSick("to slow " + QString::number(getDataValue("run grab").toInt()) + "(" + QString::number(getDataValue("run").toInt()) + ") / " + QString::number(timerTime));
        emit stop();
    }

//    data["run"] = run.elapsed();
//    if(data["run"].toInt() > timerTime)
//        this->setHealthToSick("to slow " + QString::number(data["run"].toInt()) + " / " + QString::number(timerTime));
}

void Behaviour_PipeFollowing::initPictureFolder()
{
    QDir dir( this->getSettingsValue("videoFilePath").toString());
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
    QTime blub;
    blub.restart();
//   logger->debug( "pipe angle" +QString::number(curAngle) + "°");
//   logger->debug( "pipe distance " +QString::number( distanceY ));
   addData("pipe_angle",curAngle);
   addData("pipe_distance",distanceY);
//   data["pipe_angle"] = curAngle;
//   data["pipe_distance"]= distanceY;

   if(fabs(Behaviour_PipeFollowing::curAngle) > Behaviour_PipeFollowing::deltaAngPipe)
   {
       ctrAngleSpeed = Behaviour_PipeFollowing::kpAngle * Behaviour_PipeFollowing::curAngle / 90.0;
   }

   if(fabs(Behaviour_PipeFollowing::distanceY) > Behaviour_PipeFollowing::deltaDistPipe)
   {
       ctrAngleSpeed += Behaviour_PipeFollowing::kpDist * Behaviour_PipeFollowing::distanceY / Behaviour_PipeFollowing::maxDistance;
   }

   emit angularSpeed(ctrAngleSpeed);
   emit forwardSpeed(this->constFWSpeed);
   addData("angular_speed",ctrAngleSpeed);
   addData("forward_speed",this->constFWSpeed);
   addData("intersect_y",potentialY);
//   tcl->setAngularSpeed(ctrAngleSpeed);
//   tcl->setForwardSpeed(this->constFWSpeed);
//   data["angular_speed"] = ctrAngleSpeed;
//   data["forward_speed"] = this->constFWSpeed;
//   data["intersect_y"] = potentialY;

   addData("run p-contr",blub.elapsed());
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
        if ( 'q' == waitKey( 100 ) )
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
        int thresh = getSettingsValue( "threshold", 100 ).toInt();
        threshold( displayFrame, threshImg, thresh, 255, THRESH_BINARY);
//        medianBlur( displayFrame, displayFrame, 5 );
        double width = getSettingsValue( "camWidth", 100 ).toDouble();
        double height = getSettingsValue( "camHeight", 100 ).toDouble();
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
//                logger->debug("rho " + QString::number(lines[i][0]));
//                logger->debug("theta " + QString::number(lines[i][1]));
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

//            logger->debug("****************");
//            logger->debug("= " +QString::number(avRho) + " " + QString::number(avTheta));
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
//            logger->debug("Class 1 " + QString::number(avRhoClass1) + " " + QString::number(avThetaClass1));
//            logger->debug("Class 2 " + QString::number(avRhoClass2) + " " + QString::number(avThetaClass2));
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
            if(noPipeCnt > this->getSettingsValue("badFrames").toInt())
            {
                this->setHealthToSick("20 frames without pipe");
                //TODO 180 drehen?
                this->stop();

            }
        }
        else this->noPipeCnt = 0;

//        data["rohrRho"] =  avRho; //avRhoClass1;
//        data["rohrTheta"] = avTheta; //avThetaClass1;

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
//            logger->debug("Schnittpunkt " + QString::number(intersect.x) + " " + QString::number(intersect.y));
            circle(frame,intersect,3,Scalar(255,0,255),3,8);
            circle(frame,robCenter,3,Scalar(255,0,255),3,8);

        }



        /* Ideallinie zeichnen */
        line( displayFrame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(255,0,0), 3, 8 );

        /* ins Qt Widget malen */
//        emit printFrameOnUi( displayFrame );

        if(debug)
        {
            imshow("image",frame);
            waitKey();
        }
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


//    data["nullabstand"] = d;
//    distanceY = (pt1.x - robCenter.x) * (pt1.y - pt2.y) + (robCenter.y - pt1.y) * (pt1.x - pt2.x);
}

void Behaviour_PipeFollowing::updateData()
{
    addData("current Angle",this->curAngle);

    addData("current Angle",this->curAngle);
    addData("intersect.x",this->intersect.x);
    addData("intersect.y",this->intersect.y);
    addData("distanceY",this->distanceY);
    addData("deltaDistPipe",this->deltaDistPipe);
    addData("deltaAnglePipe",this->deltaAngPipe);
    addData("kpAngle",this->kpAngle);
    addData("kpDist",this->kpDist);
    addData("robCenter.x",this->robCenter.x);
    addData("robCenter.y",this->robCenter.y);
    addData("potential Vector",this->potentialVec);

//    data["current Angle"] = this->curAngle;
//    data["intersect.x"] = this->intersect.x;
//    data["intersect.y"] = this->intersect.y;
//    data["distanceY"] = this->distanceY;
//    data["deltaDistPipe"] = this->deltaDistPipe;
//    data["deltaAnglePipe"] = this->deltaAngPipe;
//    data["kpAngle"] = this->kpAngle;
//    data["kpDist"] = this->kpDist;
//    data["robCenter.x"] = this->robCenter.x;
//    data["robCenter.y"] = this->robCenter.y;
//    data["potential Vector"] = this->potentialVec;
       emit dataChanged( this );
}

void Behaviour_PipeFollowing::updateFromSettings()
{
    qDebug() << "update Settings pipefollow";
    this->moduleMutex.lock();
    this->firstRun = 1;
    this->timerTime = this->getSettingsValue("timer",0).toInt();
    this->threshSegmentation = this->getSettingsValue("threshold",188).toInt();
    this->debug = this->getSettingsValue("debug",0).toInt();
    this->deltaAngPipe = this->getSettingsValue("deltaAngle",11).toFloat();
    this->deltaDistPipe = this->getSettingsValue("deltaDist",100).toFloat();
    this->kpDist = this->getSettingsValue("kpDist",1).toFloat();
    this->kpAngle = this->getSettingsValue("kpAngle",1).toFloat();
    this->constFWSpeed = this->getSettingsValue("fwSpeed",0.8).toFloat();
    this->robCenter = Point(this->getSettingsValue("robCenterX",320).toDouble(),this->getSettingsValue("robCenterY",240).toDouble());
    this->maxDistance = this->getSettingsValue("maxDistance",320).toFloat();
    this->moduleMutex.unlock();

//    this->timerTime = this->getSettings().value("timer",0).toInt();
//    this->threshSegmentation = this->getSettings().value("threshold",188).toInt();
//    this->debug = this->getSettings().value("debug",0).toInt();
//    this->deltaAngPipe = this->getSettings().value("deltaAngle",11).toFloat();
//    this->deltaDistPipe = this->getSettings().value("deltaDist",100).toFloat();
//    this->kpAngle = this->getSettings().value("kpDist",1).toFloat();
//    this->kpDist = this->getSettings().value("kpAngle",1).toFloat();
//    this->constFWSpeed = this->getSettings().value("fwSpeed",0.8).toFloat();
//    this->robCenter = Point(this->getSettings().value("robCenterX",320).toDouble(),this->getSettings().value("robCenterY",240).toDouble());
//    this->maxDistance = this->getSettings().value("maxDistance",320).toFloat();
}

void Behaviour_PipeFollowing::medianFilter(float &rho, float &theta)
{
//    logger->debug("in median: rho " +QString::number(rho));
//    logger->debug("in median: theta " +QString::number(theta));

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

void Behaviour_PipeFollowing::convertColor(Mat &frame, Mat &convFrame)
{
    int farbraum =  this->getSettingsValue("convColor").toInt();
    addData("farbraum",farbraum);
//    data["farbraum"] = this->getSettings().value("convColor");
    if(farbraum == 4)
    {
        convFrame.create(frame.rows,frame.cols,CV_8UC3);
        cvtColor(frame,convFrame,CV_RGB2GRAY);
    }
    else if(farbraum == 0)
    {
        convFrame.create(frame.rows,frame.cols,CV_8UC3);
        cvtColor(frame,convFrame,CV_RGB2GRAY);
    }
    else
    {
        Mat frameHSV,h,s,v;
        convFrame.create(frame.rows,frame.cols,CV_8UC1);

        cvtColor(frame,frameHSV,CV_RGB2HSV);
        h.create(frame.rows,frame.cols,CV_8UC1);
        s.create(frame.rows,frame.cols,CV_8UC1);
        v.create(frame.rows,frame.cols,CV_8UC1);

        if(farbraum == 1)
        {

            Mat out[] = {convFrame,s,v};
            split(frameHSV,out);
        }
        else if(farbraum == 2)
        {
            Mat out[] = {h,convFrame,v};
            split(frameHSV,out);
        }
        else
        {
            Mat out[] = {h,s,convFrame};
            split(frameHSV,out);
        }



        //    for (int i = 0; i < frame.rows; i++)
//    {
//        for (int j = 0; j < frame.cols; j++)
//        {
//            Vec<unsigned char, 3> hsvV = frameHSV.at<Vec<unsigned char, 3> >(i, j);
//            if(this->getSettings().value("convColor").toInt() == 1)
//                convFrame.at<unsigned char>(i, j) = hsvV[0];
//            else if(this->getSettings().value("convColor").toInt() == 2)
//                convFrame.at<unsigned char>(i, j) = hsvV[1];
//            else if(this->getSettings().value("convColor").toInt() == 3)
//                convFrame.at<unsigned char>(i, j) = hsvV[2];
//        }
//    }
    }
}

void Behaviour_PipeFollowing::moments( Mat &frame)
{
    QTime runningTime;
    QTime blub;

//    runningTime.start();
    runningTime.restart();
Mat gray;
blub.restart();
convertColor(frame,gray);
addData("run clr",blub.elapsed());
    //    equalizeHist(gray,gray);
blub.restart();
threshold(gray,gray,this->threshSegmentation,255,THRESH_BINARY);
addData("run seg",blub.elapsed());
//    threshold(gray,gray,this->getSettings().value("threshold").toInt(),255,THRESH_BINARY);
//imshow("blub",gray);
blub.restart();
    int sum;
    this->countPixel(gray,sum);
addData("run countPix",blub.elapsed());

    if( sum > 10000 )
    {
        noPipeCnt = 0;
        addData("pipe_area",sum);
//        data["pipe_area"] = sum;

//        emit dataChanged( this );
//        imshow("Dummy",gray);
        blub.restart();
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
//        data["rohrTheta"] = theta;
        if(theta < CV_PI/2)
            theta += CV_PI;
        else if(theta > CV_PI/2)
            theta -= CV_PI;

        if(theta > CV_PI/2)
            theta -= CV_PI;
        else if(theta < -CV_PI/2)
            theta += CV_PI;


        addData("run moments",blub.elapsed());
        blub.restart();
        Point pt1 = Point(m10,m01);
        Point pt2 = Point(m10 + cos(theta)*200, m01 + sin(theta)*200);
        /* Ideallinie zeichnen */
        line( frame,Point(frame.cols/2,0.0) , Point(frame.cols/2,frame.rows), Scalar(255,0,0), 3, 8 );
        line(frame,Point(m10,m01),Point(m10 + cos(theta)*200, m01 + sin(theta)*200),Scalar(255,0,0),4,CV_FILLED);
        circle(frame,robCenter,3,Scalar(255,0,255),3,8);

        addData("run paint",blub.restart());

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
//        data["rohrThetaMod"] = theta;



        //    imshow("image",frame);

        Behaviour_PipeFollowing::compIntersect(pt1,pt2);

        int nMilliseconds = runningTime.elapsed();
        addData("run bv",nMilliseconds);
//        data["runningTime"] = nMilliseconds;
        Behaviour_PipeFollowing::updateData();
//        emit printFrameOnUi(frame);
    }
    else
    {
        noPipeCnt++;
        blub.restart();
        emit forwardSpeed(this->constFWSpeed / 2);
        emit angularSpeed(0.0);
        addData("run tcl badFrame",blub.elapsed());
//        tcl->setForwardSpeed( this->getSettings().value("fwSpeed").toFloat() / 2 );
//        tcl->setAngularSpeed( .0 ); // wegen anschließendem drehen / Ballverfolgen !!!
        if(noPipeCnt > this->getSettingsValue("badFrames").toInt())
        {
            this->stop();
            this->setHealthToSick("no pipe");
        }
//        emit printFrameOnUi(frame);
    }
    blub.restart();
    dataLockerMutex.lock();
    frame.copyTo(displayFrame);
    dataLockerMutex.unlock();
    addData("run framecpy",blub.elapsed());
}

void Behaviour_PipeFollowing::grabFrame(cv::Mat &frame)
{
    dataLockerMutex.lock();
    displayFrame.copyTo(frame);
    dataLockerMutex.unlock();
}
