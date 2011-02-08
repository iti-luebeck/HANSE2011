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

    logger = Log4Qt::Logger::logger("EchoSounderForm");

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

    ui->port->setText(echo->getSettingsValue("serialPort").toString());
    ui->echoRange->setText(echo->getSettingsValue("range").toString());


    ui->sourceFile->setChecked(echo->getSettingsValue("readFromFile").toBool());
    ui->sourceSerial->setChecked(!echo->getSettingsValue("readFromFile").toBool());
    ui->fileName->setText(echo->getSettingsValue("filename").toString());
    ui->enableRecording->setChecked(echo->getSettingsValue("enableRecording").toBool());
    ui->formatCSV->setChecked(echo->getSettingsValue("formatCSV").toBool());
    ui->format852->setChecked(!echo->getSettingsValue("formatCSV").toBool());
    ui->recordFile->setText(DataLogHelper::getLogDir()+"echolog.XXX");
    ui->updateView->setChecked(false);
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
        scene.addLine(0,(i*faktor),ui->graphicsView->width(),(i*faktor),QPen(QColor(43,43,43,255)))->setZValue(10);

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

void EchoSounderForm::on_save_clicked()
{
    echo->setSettingsValue("serialPort",ui->port->text());
    echo->setSettingsValue("range",ui->range->text());
    echo->reset();

    // Hier fehlt noch ein löschen der view, wenn die range geändert wird.

}


void EchoSounderForm::on_applyButton_clicked()
{
    echo->setSettingsValue("readFromFile", ui->sourceFile->isChecked());
    echo->setSettingsValue("filename", ui->fileName->text());
    echo->setSettingsValue("enableRecording", ui->enableRecording->isChecked());
    echo->setSettingsValue("formatCSV", ui->formatCSV->isChecked());
    echo->reset();

}

void EchoSounderForm::on_selFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Echo Recording", ui->fileName->text(), "Recording (*.852)");

    if (fileName.length()>0)
        ui->fileName->setText(fileName);
}
