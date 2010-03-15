#include "trainingwindow.h"
#include "ui_trainingwindow.h"

#include <QFileDialog>

TrainingWindow::TrainingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrainingWindow)
{
    ui->setupUi(this);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx)"));
}
