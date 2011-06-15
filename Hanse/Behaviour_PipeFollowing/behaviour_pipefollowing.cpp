#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_PipeFollowing/pipefollowingform.h>
#include <opencv/cxcore.h>
#include <Module_Simulation/module_simulation.h>
#include <Framework/Angles.h>

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Webcams *cam, Module_Simulation *sim) :
        RobotBehaviour(id)
{
    this->tcl = tcl;
    this->cam = cam;
    this->sim = sim;
    setEnabled(false);
    timer.moveToThread(this);

}

bool Behaviour_PipeFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_PipeFollowing::init()
{
    logger->debug("pipe init");
    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    frame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    displayFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    segmentationFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC1 );
    toSlowCnt = 0;
    this->noPipeCnt = 0;

    this->updateFromSettings();
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    /* connect simulation */
    connect(this,SIGNAL(requestBottomFrame()),sim,SLOT(requestBottomImageSlot()));
    connect(sim,SIGNAL(newBottomImageData(cv::Mat)),this,SLOT(simFrame(cv::Mat)));

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));

}

void Behaviour_PipeFollowing::startBehaviour()
{
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }

    this->reset();
    logger->info("Behaviour started" );
    Behaviour_PipeFollowing::updateFromSettings();
    this->setHealthToOk();
    setEnabled(true);
    timer.start(timerTime);
    emit started(this);
}

void Behaviour_PipeFollowing::stop()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    timer.stop();
    if (this->isActive())
    {
        logger->info( "Behaviour stopped" );
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
    }
}

void Behaviour_PipeFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();

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
    ret.append(sim);
    return ret;
}

QWidget* Behaviour_PipeFollowing::createView(QWidget* parent)
{
    return new PipeFollowingForm( parent, this);
}

void Behaviour_PipeFollowing::simFrame(cv::Mat simFrame)
{
  //  logger->debug(" simu data");
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    //  QMutexLocker l(&this->dataLockerMutex);
    this->dataLockerMutex.lock();
    simFrame.copyTo(frame);
    this->dataLockerMutex.unlock();
//    imshow("blub",frame);
    timerSlotExecute();
}

void Behaviour_PipeFollowing::timerSlot()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    if(sim->isEnabled())
    {
        emit requestBottomFrame();
    }
    else
    {
        cam->grabBottom(frame);
        QTime blub;
        blub.restart();
        cam->grabBottom( frame );
        addData("run grab",blub.elapsed());

        timerSlotExecute();
    }
}

void Behaviour_PipeFollowing::timerSlotExecute()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    QTime run;
    run.restart();

    if(!frame.empty()) {
        this->setHealthToOk();
        Behaviour_PipeFollowing::moments(frame);
        addData("time total", run.elapsed());
        if (isEnabled() && this->getHealthStatus().isHealthOk()) {
            Behaviour_PipeFollowing::controlPipeFollow();
        }
    } else {
        this->setHealthToSick("empty frame");
    }

    if(getDataValue("time total").toInt() > timerTime) {
        logger->error("pipefollow too slow");
        toSlowCnt++;
        addData("err toSlow",toSlowCnt);
    }
}

void Behaviour_PipeFollowing::controlPipeFollow()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    float ctrAngleSpeed = 0.0;
    //   logger->debug( "pipe angle" +QString::number(curAngle) + "°");
    //   logger->debug( "pipe distance " +QString::number( distanceY ));
    addData("pipe_angle",curAngle);
    addData("pipe_distance",distanceY);

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

    emit dataChanged( this );
}

void Behaviour_PipeFollowing::analyzeVideo()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    QString videoFile = getSettingsValue("video directory").toString();

    QDir dir( videoFile );
    dir.setFilter( QDir::Files );
    QStringList filters;
    filters << "*.jpg";
    dir.setNameFilters( filters );
    files = dir.entryList();
    Mat frame;

    for ( int i = 0; i < files.count(); i++ )
    {
        QString filePath = videoFile;
        filePath.append( "/" );
        filePath.append( files[i] );
        frame = imread( filePath.toStdString() );

        Behaviour_PipeFollowing::moments(frame);
        Behaviour_PipeFollowing::updateData();

        controlPipeFollow();

        msleep(500);
    }
}

void Behaviour_PipeFollowing::compIntersect(Point pt1, Point pt2)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

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

    emit dataChanged( this );
}

void Behaviour_PipeFollowing::updateFromSettings()
{
    logger->debug("update Settings pipefollow");
    this->dataLockerMutex.lock();
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
    this->dataLockerMutex.unlock();
}

void Behaviour_PipeFollowing::convertColor(Mat &frame, Mat &convFrame)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    int farbraum =  this->getSettingsValue("convColor").toInt();
    addData("farbraum",farbraum);
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
    }
}

void Behaviour_PipeFollowing::moments( Mat &frame)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    QTime blub;
    blub.restart();
    Mat gray;

    // Perform thresholding with automated threshold selection.
    convertColor(frame,gray);
    threshold(gray, gray, this->threshSegmentation, 255, THRESH_BINARY | THRESH_OTSU);

    addData("time segmentation", blub.elapsed());
    blub.restart();

    // Initialize the moment structure.
    IplImage *ipl = new IplImage(gray);
    CvMoments M;
    cvMoments(ipl, &M, 1);

    // Get number of pipe pixels.
    blub.restart();
    double area = cvGetSpatialMoment(&M, 0, 0);
    addData("time moments", blub.elapsed());

    if (area > 10000) {
        noPipeCnt = 0;
        addData("pipe area", area);

        // First order moments -> mean position
        double m10 = cvGetSpatialMoment( &M, 1, 0 ) / area;
        double m01 = cvGetSpatialMoment( &M, 0, 1 ) / area;

        // Second order moments -> orientation
        double mu11 = cvGetCentralMoment( &M, 1, 1 ) / area;
        double mu20 = cvGetCentralMoment( &M, 2, 0 ) / area;
        double mu02 = cvGetCentralMoment( &M, 0, 2 ) / area;
        double theta = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

        // Theta is now the rotation angle reletive to the x axis.
        // We want it relative to the y axis -> 90° ccw
        theta -= CV_PI / 2;
        if (theta < -CV_PI) {
            theta += CV_PI;
        }

        if (theta < -CV_PI/2) {
            theta += CV_PI;
        } else if (theta > CV_PI/2) {
            theta -= CV_PI;
        }

        addData("time moments", blub.elapsed());

        blub.restart();
        Point pt1 = Point(m10, m01);
        Point pt2 = Point(m10 + 200 * cos(theta), m01 + 200 * sin(theta));

        // Draw ideal line.
        line(frame, Point(frame.cols / 2, 0.0), Point(frame.cols / 2, frame.rows), Scalar(255,0,0), 3, 8);
        line(frame, Point(m10, m01), Point(m10 + 200 * sin(theta), m01 - 200 * cos(theta)), Scalar(255,0,0), 4, CV_FILLED);
        circle(frame, robCenter, 3, Scalar(255,0,255), 3, 8);

        curAngle = Angles::pi2deg(theta);

        Behaviour_PipeFollowing::compIntersect(pt1,pt2);
        Behaviour_PipeFollowing::updateData();
    }
    else
    {
        noPipeCnt++;
        blub.restart();
        emit forwardSpeed(this->constFWSpeed / 2);
        emit angularSpeed(0.0);
        addData("run tcl badFrame",blub.elapsed());
        if(noPipeCnt > this->getSettingsValue("badFrames").toInt())
        {
            this->stop();
            this->setHealthToSick("no pipe");
        }
    }
    blub.restart();
    dataLockerMutex.lock();
    frame.copyTo(displayFrame);
    gray.copyTo(segmentationFrame);
    dataLockerMutex.unlock();
    addData("run framecpy",blub.elapsed());
}

void Behaviour_PipeFollowing::grabFrame(cv::Mat &frame)
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    dataLockerMutex.lock();
    if(getSettingsValue("frameOutput").toBool())
        displayFrame.copyTo(frame);
    else
    {
        cv::Mat tmp(WEBCAM_HEIGHT,WEBCAM_WIDTH,CV_8UC3);
        cvtColor(segmentationFrame,tmp,CV_GRAY2RGB);
        tmp.copyTo(frame);
    }
    dataLockerMutex.unlock();
}

void Behaviour_PipeFollowing::setUpdatePixmapSlot(bool bol){
    emit setUpdatePixmapSignal(bol);
}

void Behaviour_PipeFollowing::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(stop()));
    }
}
