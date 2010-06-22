#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    videoFile = "../../pipe_handy.avi";
    videoFile = "Images/Camera_Stills/pipe1.png";
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
void MainWindow::on_loadVideoButton_clicked()
{
    videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.png *.avi *.mpg *.divx)"));

}

void MainWindow::on_loadPictureButton_clicked()
{
//    videoFile = QFileDialog::getOpenFileName(this, tr("Open Picture"), "", tr("Pictures (*.png *.jpg *.jpeg *.bmp)"));
    pipeFollow.setDebug(ui->debugCheckBox->isChecked());
    pipeFollow.setThresh(ui->segmentationThreshLineEdit->text().toInt());
    pipeFollow.findPipeInPic(videoFile);
}


void MainWindow::on_detLineButton_clicked()
{
   pipeFollow.setDebug(ui->debugCheckBox->isChecked());
   pipeFollow.setThresh(ui->segmentationThreshLineEdit->text().toInt());
 pipeFollow.findPipe(videoFile);

//    pipeFollow.findPipeInPic(videoFile);
//    qDebug() << "searching Lines \n\r";


}
