#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv/highgui.h>
#include <opencv/cxcore.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::runFrame()
{
    this->getFrame(frame);
    QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
    ui->frameLabel->setPixmap(QPixmap::fromImage(image1));
}

void MainWindow::getFrame(Mat &frame)
{
    IplImage* frame2 = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);
    vi.getPixels(this->camID, (unsigned char *)frame2->imageData, true, true);
    frame = Mat(frame2,false);

}

void MainWindow::on_startButton_clicked()
{
    this->camID = ui->camIDLineEdit->text().toInt();

    vi.setIdealFramerate(this->camID,30);
    vi.setupDevice(this->camID,640,480);
    timer.start(200);
}


void MainWindow::on_settingsButton_clicked()
{
    vi.listDevices(false);
    vi.showSettingsWindow(this->camID);
}

void MainWindow::on_stopButton_clicked()
{
    timer.stop();
    vi.stopDevice(this->camID);
}

void MainWindow::on_saveApplybutton_clicked()
{
    this->camID = ui->camIDLineEdit->text().toInt();
}

void MainWindow::timerSlot()
{
    this->runFrame();
}
