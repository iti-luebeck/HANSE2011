#include "echosounderform.h"
#include "ui_echosounderform.h"
#include "echoreturndata.h"
#include <Framework/dataloghelper.h>

EchoSounderForm::EchoSounderForm(Module_EchoSounder* echo, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EchoSounderForm),
    scene()
{
    ui->setupUi(this);
    this->echo = echo;
    this->ui->graphicsView->setScene(&scene);

    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("black"));
    gi.setColorAt(279,QColor("black"));
    for(int i =0; i<ui->graphicsView->width()/1; i++)
    {
//        QGraphicsRectItem *rit = scene.addRect((i*10),1,(i+1)*10,274,(Qt::NoPen));
//        rit->setBrush(QBrush(gi));
//        ritQueue.append(rit);
        dataQueue.append(gi);
    }
    ui->readFileox->setChecked(echo->getSettingsValue("readFromFile").toBool());
    ui->updateView->setChecked(false);
    ui->port->setText(echo->getSettingsValue("serialPort").toString());
    QObject::connect(echo,SIGNAL(newEchoData(EchoReturnData)),this,SLOT(updateSounderView(EchoReturnData)));

}

EchoSounderForm::~EchoSounderForm()
{
    delete ui;
}

void EchoSounderForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void EchoSounderForm::updateSounderView(const EchoReturnData data)
{

    float n = data.getEchoData().length();

    float range = data.getRange();
    scene.clear();
    int height = ui->graphicsView->height()-1;
    int faktor = (height / range);

//    scene.addLine(-256,280,256,280,QPen(QColor("red")))->setZValue(10);
    for(int i = 1; i < range+1; i++)
    {
        scene.addLine(0,(i*faktor),ui->graphicsView->width(),(i*faktor),QPen(QColor("red")))->setZValue(10);
    }

    if (ui->updateView->isChecked())
    {

        QLinearGradient gi(0,0,0,279);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            gi.setColorAt(1.0*i/n,QColor(0,2*b,0));
        }
        dataQueue.append(gi);
        dataQueue.pop_front();

        for(int i =0; i<ui->graphicsView->width()/1; i++)
        {
            QGraphicsRectItem *rit = scene.addRect((i*1),1,(i+1)*1,274,(Qt::NoPen));
            rit->setBrush(QBrush(dataQueue[i]));
        }
    }

}

void EchoSounderForm::on_onSelFileClicked_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Sonar Recording", ui->fileName->text(), "Recording (*.852)");

    if (fileName.length()>0)
        ui->fileName->setText(fileName);
}

void EchoSounderForm::on_applyButton_clicked()
{
    echo->setSettingsValue("readFromFile",ui->readFileox->isChecked());
//    ui->readFileox->setChecked(true);
    if(ui->readFileox->isChecked())
        this->on_onSelFileClicked_clicked();
    echo->setSettingsValue("filename",ui->fileName->text());
    echo->setSettingsValue("serialPort",ui->port->text());
    echo->reset();

}

void EchoSounderForm::on_readFileox_clicked(bool checked)
{
    echo->setSettingsValue("readFromFile",checked);
}
