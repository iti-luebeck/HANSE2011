#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <opencv/highgui.h>
#include "sonarechofilter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //define Actions for classification
    actionPos = new QAction(this);
    actionPos->setShortcut(Qt::Key_1);
    connect(actionPos,SIGNAL(triggered()),this,SLOT(positivSample()));
    actionNeg = new QAction(this);
    actionNeg->setShortcut(Qt::Key_2);
    connect(actionNeg,SIGNAL(triggered()),this,SLOT(negativSample()));
    actionSkip = new QAction(this);
    actionSkip->setShortcut(Qt::Key_3);
    connect(actionSkip,SIGNAL(triggered()),this,SLOT(skipSample()));
    //graphic for polarcoordinates
    this->ui->graphicsView_2->setScene(&scene2);
    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("black"));
    gi.setColorAt(1,QColor("black"));
    simpleViewWidth = 51;
    selSampleWidth = ui->selSampleWidthSlider->value();
    scene2.clear();
    dataQueue.clear();
    for(int i =0; i<simpleViewWidth; i++)
        dataQueue.append(gi);
    //init variables
    viewData.clear();
    samples.clear();
    wallCandidates.clear();
    pSamples.clear();
    nSamples.clear();
    range = 0.0;
    currSample = 0;
    filter = NULL;

    svm = new SVMClassifier();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_saveSVM_clicked()
{
    svm->saveClassifier("SLtrainingSVM");
}

void MainWindow::on_loadSVM_clicked()
{
    svm->loadClassifier("SLtrainingSVM");
}

void MainWindow::updateSonarView2(const QList<QByteArray> curDataSet)
{
    float n = curDataSet.first().length();
    scene2.clear();
    int height = ui->graphicsView_2->height()-1;
    float faktor = (height/range);
//    if(range > 20.0)
//        faktor = faktor * 10;
    if(true)
    {

        for(int i = 1; i<range+1; i++)
            scene2.addLine(0,(i*faktor),simpleViewWidth,(i*faktor),QPen(QColor(200,83,83,255)))->setZValue(10);

        for(int j=0; j<curDataSet.size(); j++)
        {
            QLinearGradient gi(0,0,0,279);
            for (int i = 0; i < n; i++) {
                QByteArray data = curDataSet.at(j);
                char b = data[i];
//                if(j == (curDataSet.length()/2))
                int wws = ui->wallwindowsize->text().toInt();
                if((j == (currSample)))
                {
                     if((i >= (wallCandidates.first() - wws)) && (i <= (wallCandidates.first() + wws)))
                        gi.setColorAt(1.0*i/n,QColor(255*b,0,0));
                     else
                         gi.setColorAt(1.0*i/n,QColor(0,0,255*b));
                }
                else
                    gi.setColorAt(1.0*i/n,QColor(0,255*b,0));
            }
            dataQueue.append(gi);
            dataQueue.pop_front();

        }

        for(int i =0; i<currSample; i++)
            scene2.addRect(i,1,1,274,(Qt::NoPen),QBrush(dataQueue[i]));
        scene2.addRect(currSample,1,selSampleWidth,274,(Qt::NoPen),QBrush(dataQueue[currSample]));
        for(int i =currSample+1; i<simpleViewWidth; i++)
            scene2.addRect(i+selSampleWidth-1,1,1,274,(Qt::NoPen),QBrush(dataQueue[i]));

    }
}

void MainWindow::on_loadSonarFile_clicked()
{
    //    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "../bin/sonarloc/");
    //    QString path = "/home/hanse/Desktop/scanningsonar.852";
    QString path = "/home/kluessi/Downloads/untertrave.852";

    QDateTime time = QDateTime::fromString("M2d2y1114:42:59","'M'M'd'd'y'yyhh:mm:ss");
    SonarEchoFilter filter = SonarEchoFilter();

    SonarDataSourceFile *file = NULL;
    file = new SonarDataSourceFile(this,path);
    file->fileReaderDelay = 10;
    file->startTime = time;
    if(!file->isOpen())
    {
        qDebug() << "ERR could not open file";
        file = NULL;
    }
    else
    {
        samples.clear();
        wallCandidates.clear();
        SonarReturnData dat = file->getNextPacket();
        range = dat.getRange();
        int count = 222;
        currSample = simpleViewWidth/2;
        int cntWallCandSkip = 0;
        QByteArray filteredSample;
        while(dat.isPacketValid() && count > 0)
        {
            cntWallCandSkip++;
           filteredSample = filter.newSonarData(dat);
           if(cntWallCandSkip > currSample)
               wallCandidates.append(filter.findWall(dat,filteredSample));
//           wallCandidates.append(filter.findWall(dat,dat.getEchoData()));
            samples.append(filteredSample);
//           qDebug() << "size" << QString::number(filteredSample.size()) << " " << dat.getEchoData().size();
//            samples.append(dat.getEchoData());
            dat = file->getNextPacket();
            count--;
        }
        if(samples.size() != (wallCandidates.size()+currSample))
            qDebug() << "something terribly went wrong";
        qDebug() << "Samples: " << samples.length();

    }
    delete file;
    file = NULL;
}

void MainWindow::askForClasses()
{
    if(samples.size() < 50 )
    {
        qDebug() << "Need more than 50 samples to classify";
    }
    else
    {
        viewData.clear();
        for(int i=0;i<simpleViewWidth;i++)
        {
            viewData.append(samples.takeFirst());
        }
        qDebug() << "size "+QString::number(viewData.size()) << "current sample " << QString::number(currSample);
        this->updateSonarView2(viewData);

        this->addAction(actionPos);
        this->addAction(actionSkip);
        this->addAction(actionNeg);
    }
}

void MainWindow::positivSample()
{
    pSamples.append(viewData.at(currSample));
    skipSample();
}

void MainWindow::negativSample()
{
    pSamples.append(viewData.at(currSample));
    skipSample();
}

void MainWindow::skipSample()
{
    if(!samples.isEmpty())
    {
        viewData.pop_front();
        viewData.append(samples.takeFirst());
        wallCandidates.pop_front();
        this->updateSonarView2(viewData);

    }
    else
    {
     this->clearActions();
    }
}
void MainWindow::clearActions()
{
    scene2.clear();
    this->removeAction(actionPos);
    this->removeAction(actionSkip);
    this->removeAction(actionNeg);
}


void MainWindow::on_selectSamples_clicked()
{
    this->askForClasses();
}

void MainWindow::on_selSampleWidthSlider_sliderMoved(int position)
{
    ui->selSampleLabel->setText("Selected Sample Width ("+QString::number(position)+")");
    selSampleWidth = position;
}


cv::Mat MainWindow::cvtList2Mat()
{

    QByteArray array;
    cv::Mat m = cv::Mat::zeros(pSamples.size()+nSamples.size(),pSamples.first().size(),CV_32F);

    for(int j=0;j<pSamples.size();j++)
    {
        array = pSamples.at(j);
        for(int i=0;i<array.size();i++)
        {
            m.at<float>(j,i) = array[i];
        }
    }
    for(int j=0;j<nSamples.size();j++)
    {
        array = nSamples.at(j);
        for(int i=0;i<array.size();i++)
        {
            m.at<float>(j+pSamples.size(),i) = array[i];
        }
    }

    return m;
}

void MainWindow::on_testSVM_clicked()
{

    on_loadSonarFile_clicked();

//    pSamples = samples;
//    cv::Mat testData = cvtList2Mat();
//    CvMat test = testData;

    cv::Mat m = cv::Mat(samples.first().size(),1,CV_32FC1);
    QByteArray array;
    int j = 12;
//    for(int j=0;j<samples.size();j++)
//    {
        array = samples.at(j);
        for(int i=0;i<array.size();i++)
        {
            m.at<float>(0,i) = array[i];
        }
        CvMat test = (CvMat)m;
        int predClass = 9;
        predClass = svm->svmClassification(&test);
        int blub = 34;
        qDebug() << j << " Class " << predClass;

//    }




}

void MainWindow::on_trainSVM_clicked()
{
    CvMat samples;
    CvMat classes;
    cv::Mat clasLab = cv::Mat::ones(1,pSamples.size()+nSamples.size(),CV_32S);
    for(int i=0;i<nSamples.size();i++)
    {
        clasLab.at<int>(0,i+pSamples.size()) = -1;
    }

    samples = cvtList2Mat();
    classes = clasLab;
    svm->train(&samples,&classes);


}
