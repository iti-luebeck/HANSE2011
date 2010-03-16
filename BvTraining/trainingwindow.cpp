#include "trainingwindow.h"
#include "ui_trainingwindow.h"

#include <QFileDialog>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "imageprocessor.h"

using namespace cv;

TrainingWindow::TrainingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrainingWindow)
{
    ui->setupUi(this);

    videoFile = "../../front_sauce10.avi";
    blobTraining.load("bla.xml");
}

TrainingWindow::~TrainingWindow()
{
    delete ui;
}

void TrainingWindow::changeEvent(QEvent *e)
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

void TrainingWindow::on_trainButton_clicked()
{
    blobTraining.train(videoFile);
}

void TrainingWindow::on_loadButton_clicked()
{
    videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx)"));
}

void TrainingWindow::on_selectButton_clicked()
{
    blobTraining.select(videoFile);
}

void TrainingWindow::on_testButton_clicked()
{
    blobTraining.test(videoFile);
}

void TrainingWindow::on_loadBlobButton_clicked()
{
    QString loadFile = QFileDialog::getOpenFileName(this, tr("Save Classifier"), "", tr("Classifier Files (*.xml)"));
    blobTraining.load(loadFile);
}

void TrainingWindow::on_saveBlobButton_clicked()
{
    QString saveFile = QFileDialog::getSaveFileName(this, tr("Save Classifier"), "", tr("Classifier Files (*.xml)"));
    blobTraining.save(saveFile);
}
