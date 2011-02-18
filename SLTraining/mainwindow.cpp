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
    connect(&svmClassy,SIGNAL(updateUI(SonarReturnData)),this,SLOT(updateSonarView(SonarReturnData)));


    oldHeading = NAN;
    oldStepSize = 0;
    this->ui->graphicsView->setScene(&scene);
    scanLine = scene.addLine(0,0,0,50, QPen(QColor("red")));
    scanLine->setZValue(20);

    scene.addLine(-50,0,50,0,QPen(QColor("white")))->setZValue(10);
    scene.addLine(0,-50,0,50,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-50,-50,100,100,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-40,-40,80,80,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-30,-30,60,60,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-20,-20,40,40,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-10,-10,20,20,QPen(QColor("white")))->setZValue(10);

    //define QMessageBox for user interaction
    pos = box.addButton(tr("Positiv"),QMessageBox::ActionRole);
    neg = box.addButton(tr("Negativ"),QMessageBox::ActionRole);
    skip = box.addButton(tr("Skip"),QMessageBox::ActionRole);
    quit = box.addButton(tr("Quit"),QMessageBox::ActionRole);

    pos->setShortcut(Qt::Key_1);
    neg->setShortcut(Qt::Key_2);
    skip->setShortcut(Qt::Key_3);
    quit->setShortcut(Qt::Key_Q);

    box.setInformativeText("How to classify actual EchoData?");
    box.setEscapeButton(quit);
    box.setDefaultButton(skip);
    box.setIcon(QMessageBox::Question);





    this->ui->graphicsView_2->setScene(&scene2);

    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("black"));
    gi.setColorAt(279,QColor("black"));
    for(int i =0; i<100; i++)
    {
//        QGraphicsRectItem *rit = scene.addRect((i*10),1,(i+1)*10,274,(Qt::NoPen));
//        rit->setBrush(QBrush(gi));
//        ritQueue.append(rit);
        dataQueue.append(gi);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loadSamples_clicked()
{
    QString s = "U";
//    QString s = QFileDialog::getOpenFileName(this, tr("Open File"), "../bin/sonarloc/");

     svmClassy.loadSamples(s);
}

void MainWindow::on_saveSVM_clicked()
{
    svmClassy.save();
}

void MainWindow::on_loadSVM_clicked()
{
    svmClassy.load();
}

void MainWindow::updateSonarView2(const SonarReturnData data)
{
    float range  = data.getRange();

    float n = data.getEchoData().length();
    scene2.clear();
    int height = ui->graphicsView_2->height()-1;
    float faktor = (height / range);
    if(range >= 20.0){
        faktor = faktor * 10;
    }
//    scene.addLine(-256,280,256,280,QPen(QColor("red")))->setZValue(10);


    if (ui->checkBox_3->isChecked())
    {
        for(int i = 1; i < range+1; i++)
        {

            scene2.addLine(0,(i*faktor),100,(i*faktor),QPen(QColor(200,83,83,255)))->setZValue(10);
        }

        QLinearGradient gi(0,0,0,279);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            gi.setColorAt(1.0*i/n,QColor(0,2*b,0));
        }
        dataQueue.append(gi);
        dataQueue.pop_front();

        for(int i =0; i<100; i++)
        {
            QGraphicsRectItem *rit = scene2.addRect((i*1),1,(i+1)*1,274,(Qt::NoPen));
            rit->setBrush(QBrush(dataQueue[i]));
        }
    }

}

void MainWindow::updateSonarView(const SonarReturnData data)
{
    float n = data.getEchoData().length();
    float range = data.getRange();

    if (oldStepSize != data.switchCommand.stepSize) {
        oldStepSize = data.switchCommand.stepSize;
        foreach (QGraphicsPolygonItem* o, queue) {
            delete o;
        }
        queue.clear();
    }


    ui->time->setDateTime(data.switchCommand.time);
    ui->heading->setText(QString::number(data.getHeadPosition()));
    ui->gain->setText(QString::number(data.switchCommand.startGain));
    ui->range->setText(QString::number(data.getRange()));

    // TODO: if any of the parameters have changed; reset the scene

    float newHeading = data.getHeadPosition();
    int bla = oldHeading;
    bool isnumber = (bla != 0);

    if(ui->checkBox_3->isChecked() && isnumber && (fabs(newHeading - oldHeading)<20 || fabs(newHeading - oldHeading)>340))
    {

        QPolygonF polygon;

        QPointF endPoint1 = QTransform().rotate(oldHeading).map(QPointF(range,0));
        QPointF endPoint2 = QTransform().rotate(newHeading).map(QPointF(range,0));

        polygon << QPointF(0,0) << endPoint1 << endPoint2;
        QGraphicsPolygonItem *it = scene.addPolygon(polygon,QPen(Qt::NoPen));
        queue.append(it);

        scanLine->setRotation(newHeading-90);

        QLinearGradient g(QPointF(0, 0), endPoint2);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            g.setColorAt(1.0*i/n,QColor(0,2*b,0));
        }
        it->setBrush(QBrush(g));

        // this should ensure that a full circle is conserved even at highest resolution
        // it may result in overlay, but this doesn't matter since newer items will always
        // be drawn on top of older ones.
        // 480: don't ask, it just works :)
        while (queue.size()>480/(oldStepSize*3+3)) {
            delete queue.takeFirst();
        }

    }

    oldHeading = newHeading;
}

void MainWindow::on_loadSonarFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Open File"), "../bin/sonarloc/");
      svmClassy.readSonarFile(s);
  }


void MainWindow::askForClasses()
{
//    QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "../bin/sonarloc/");
    QString path = "/home/hanse/Desktop/scanningsonar.852";

    QList<QByteArray> pSamples;
    QList<QByteArray> nSamples;
    SonarDataSourceFile *file = NULL;

    QDateTime time = QDateTime::fromString("M2d2y1114:42:59","'M'M'd'd'y'yyhh:mm:ss");
    file = new SonarDataSourceFile(this,path);
    file->fileReaderDelay = 10;
    file->startTime = time;

//    filter = new SonarEchoFilter();

    SonarEchoFilter filter = SonarEchoFilter();

    if(!file->isOpen())
    {
        qDebug() << "ERR could not open file";
        file = NULL;
    }

    bool check = false;
    SonarReturnData dat = file->getNextPacket();

    QByteArray arr = filter.newSonarData(dat);
    QByteArray tmp = dat.packet;
    for(int i = 12; i < arr.size()+12; i++)
        tmp[i] = arr[i-12];

    SonarReturnData data = SonarReturnData(dat.switchCommand,tmp);

    while(data.isPacketValid() && !check)
    {
        this->updateSonarView(data);
        this->updateSonarView2(data);

        if(ui->classify->isChecked())
        {
        box.exec();
        if(box.clickedButton() == quit)
            check = true;
        else if(box.clickedButton() == pos)
            pSamples.append(data.getEchoData());
        else  if(box.clickedButton() == neg)
            nSamples.append(data.getEchoData());
    } else
        {

    }
        data = file->getNextPacket();
    }


}



void MainWindow::on_selectSamples_clicked()
{
    this->askForClasses();
}
