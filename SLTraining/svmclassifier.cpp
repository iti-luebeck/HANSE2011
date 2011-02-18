#include "svmclassifier.h"
#include <QDebug>

SVMClassifier::SVMClassifier(QObject *parent) : QObject(parent)
{
    svm = new CvSVM();
    file = NULL;
}

SVMClassifier::~SVMClassifier()
{
    delete(svm);
}

void SVMClassifier::loadSamples(QString path)
{
    CvMat* data = cvCreateMat(5,9,CV_32FC1);
    CvMat* label = cvCreateMat(5,1,CV_32FC1);

//    FileStorage storage("../Hanse/bin/sonarloc/myData.xml", CV_STORAGE_WRITE);
//    storage.writeObj("trainingSamples", data);
//    storage.writeObj("trainingLabels", label);
//    storage.release();

    CvFileStorage * fs = cvOpenFileStorage( "../Hanse/bin/sonarloc/myData.xml", 0, CV_STORAGE_READ );
    CvMat * training = (CvMat *)cvReadByName(fs, 0, "trainingSamples", 0);
    CvMat * labels = (CvMat *)cvReadByName(fs, 0, "trainingLabels", 0);
    float gam = 1.0/(training->rows);
    svmParam.gamma = gam;
    qDebug() << gam;
//    fs = cvOpenFileStorage( "../bin/sonarloc/myData2.xml", 0, CV_STORAGE_WRITE );
//        cvWrite(fs, "trainingSamples", training, cvAttrList(0,0));
//        cvWrite(fs, "traininglabels", labels, cvAttrList(0,0));

    svm->train(training, labels, 0,0, svmParam);
}

void SVMClassifier::save()
{
//    CvFileStorage *fs = cvOpenFileStorage("../Hanse/bin/sonarloc/svmParam.xml",0,CV_STORAGE_WRITE);
//    svm->write(fs,"svm");
    svm->save("../Hanse/bin/sonarloc/svmParam.xml");
}

void SVMClassifier::load()
{
    svm->load("../Hanse/bin/sonarloc/svmParam.xml");
}

void SVMClassifier::initSVM()
{
    svmParam.svm_type = 100;
    svmParam.kernel_type = 2;
    svmParam.degree = 3;
    svmParam.gamma = 0.125;
    svmParam.coef0 = 1;
    svmParam.nu = 0.5;
    svmParam.C = 1;
    svmParam.p = 0.1;
    svmParam.term_crit.epsilon = 0.0001;
    svmParam.term_crit.type = CV_TERMCRIT_EPS;
}

void SVMClassifier::readSonarFile(QString path)
{
    QDateTime time = QDateTime::fromString("M2d2y1114:42:59","'M'M'd'd'y'yyhh:mm:ss");
    file = new SonarDataSourceFile(this,path);
    file->fileReaderDelay = 10;
    file->startTime = time;

    if(!file->isOpen())
    {
        qDebug() << "ERR could not open file";
        file = NULL;
    }
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    timer.start(200);

}

void SVMClassifier::timerSlot()
{
    SonarReturnData data = file->getNextPacket();
    if(data.isPacketValid())
        emit updateUI(data);
}

