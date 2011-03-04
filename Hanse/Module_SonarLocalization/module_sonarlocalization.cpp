#include "module_sonarlocalization.h"
#include <QtGui>
#include "form_sonarlocalization.h"
#include "sonarechofilter.h"
#include "sonarparticlefilter.h"
#include <opencv/cv.h>
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <qfile.h>
#include <qtextstream.h>
#include <Module_ScanningSonar/sonardatasourcefile.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar, Module_PressureSensor* pressure)
    : RobotModule(id)
{
    this->sonar = sonar;
    this->pressure = pressure;
}

void Module_SonarLocalization::init()
{

//    this->filter = new SonarEchoFilter(this);
//    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), filter, SLOT(newSonarData(SonarReturnData)));

//    // run particle filter in own thread
//    this->pf = new SonarParticleFilter(this, filter);
//    qRegisterMetaType< QList<QVector2D> >("QList<QVector2D>");
//    connect(filter, SIGNAL(newImage(QList<QVector2D>)), pf, SLOT(newImage(QList<QVector2D>)));
//    pfThread.start();
//    pf->moveToThread(&pfThread);

//    connect(pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEst(QVector3D)));

}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
    filter->reset();
    pf->reset();
}

void Module_SonarLocalization::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_SonarLocalization::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonar);
    return ret;
}

QWidget* Module_SonarLocalization::createView(QWidget* parent)
{
    return new Form_SonarLocalization(parent, this);
}

void Module_SonarLocalization::initSVM()
{
    svmParam.svm_type = getSettingsValue("svm").toInt();
    svmParam.kernel_type = getSettingsValue("kernel").toInt();
    svmParam.degree = getSettingsValue("degree").toInt();
    svmParam.gamma = getSettingsValue("gamma").toDouble(); //1/num_features
    svmParam.coef0 = getSettingsValue("coef0").toDouble();
    svmParam.nu = 0.5;
    svmParam.C = 1;
    svmParam.p = 0.1;
    svmParam.term_crit.epsilon = getSettingsValue("epsilon").toDouble();
    svmParam.term_crit.type = CV_TERMCRIT_EPS;
    }

void Module_SonarLocalization::trainSVM()
{

    CvMat* data = cvCreateMat(5,9,CV_32FC1);
    CvMat* label = cvCreateMat(5,1,CV_32FC1);

//    FileStorage storage("../bin/sonarloc/myData.xml", CV_STORAGE_WRITE);
//    storage.writeObj("trainingSamples", data);
//    storage.writeObj("trainingLabels", label);
//    storage.release();

        CvFileStorage * fs = cvOpenFileStorage( "../bin/sonarloc/myData.xml", 0, CV_STORAGE_READ );
    CvMat * training = (CvMat *)cvReadByName(fs, 0, "trainingSamples", 0);
    CvMat * labels = (CvMat *)cvReadByName(fs, 0, "trainingLabels", 0);
    logger->debug(QString::number(training->rows));
    float gam = 1.0/(training->rows);
    logger->debug("GaMMA"+QString::number(gam));
    svmParam.gamma = gam;

//    fs = cvOpenFileStorage( "../bin/sonarloc/myData2.xml", 0, CV_STORAGE_WRITE );
//        cvWrite(fs, "trainingSamples", training, cvAttrList(0,0));
//        cvWrite(fs, "traininglabels", labels, cvAttrList(0,0));

    svm = new CvSVM(training,labels,0,0,svmParam);
}

SonarParticleFilter& Module_SonarLocalization::particleFilter() const
{
    return *(this->pf);
}

void Module_SonarLocalization::newPositionEst(QVector3D p)
{
    emit newLocalizationEstimate();
}

void Module_SonarLocalization::setLocalization(QVector2D position)
{
    pf->setLocalization(position);
}

Position Module_SonarLocalization::getLocalization()
{
    QVector3D p = pf->getBestEstimate();
    Position r;
    r.setX(p.x());
    r.setY(p.y());
    r.setZ(pressure->getDepth());
    r.setYaw(p.z()*180/M_PI);
    return r;
}

float Module_SonarLocalization::getLocalizationConfidence()
{
    return pf->getParticles()[0].w();
}

QDateTime Module_SonarLocalization::getLastRefreshTime()
{
    return QDateTime::currentDateTime(); // TODO
}

bool Module_SonarLocalization::isLocalizationLost()
{
    return false; // TODO must involve some kind of threshold
}
