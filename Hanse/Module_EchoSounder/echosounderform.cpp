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
    gi.setColorAt(0,QColor("white"));
    gi.setColorAt(1,QColor("white"));
    for(int i =0; i<ui->graphicsView->width()/1; i++)
    {
        //        QGraphicsRectItem *rit = scene.addRect((i*10),1,(i+1)*10,274,(Qt::NoPen));
        //        rit->setBrush(QBrush(gi));
        //        ritQueue.append(rit);
        dataQueue.append(gi);
    }

    ui->port->setText(echo->getSettingsValue("serialPort").toString());
    ui->echoRange->setText(echo->getSettingsValue("range").toString());
    ui->serialPort->setText(echo->getSettingsValue("serialPort").toString());

    ui->sourceFile->setChecked(echo->getSettingsValue("readFromFile").toBool());
    ui->sourceSerial->setChecked(!echo->getSettingsValue("readFromFile").toBool());
    ui->fileName->setText(echo->getSettingsValue("filename").toString());
    ui->enableRecording->setChecked(echo->getSettingsValue("enableRecording").toBool());
    ui->formatCSV->setChecked(echo->getSettingsValue("formatCSV").toBool());
    ui->format852->setChecked(!echo->getSettingsValue("formatCSV").toBool());
    ui->recordFile->setText(DataLogHelper::getLogDir()+"echolog.XXX");
    ui->updateView->setChecked(false);

    ui->averageWindowInput->setText(echo->getSettingsValue("averageWindow").toString());

    ui->thresholdInput->setText(echo->getSettingsValue("threshold").toString());
    ui->thresholdText->setText(echo->getSettingsValue("threshold").toString());
    ui->gainText->setText(echo->getSettingsValue("gain").toString());
    ui->gain->setText(echo->getSettingsValue("gain").toString());
    ui->timer->setText(echo->getSettingsValue("scanTimer").toString());
    ui->timerInput->setText(echo->getSettingsValue("scanTimer").toString());
    ui->factorInput->setText(echo->getSettingsValue("calcFactor").toString());
    ui->range->setText(echo->getSettingsValue("range").toString());

    QObject::connect(echo,SIGNAL(newEchoData(EchoReturnData)),this,SLOT(updateSounderView(EchoReturnData)));
    QObject::connect(echo,SIGNAL(newEchoUiData(float)),this,SLOT(updateEchoUi(float)));

    avgTemp = -1.0;
    einheit = -1.0;

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
    einheit = data.getEchoData().length()/data.getRange();
    float range;
    if(echo->getSettingsValue("readFromFile").toBool() == true){
        range = data.getRange();
    }else{
        range = echo->getSettingsValue("range").toFloat();
    }


    int threshold = echo->getSettingsValue("threshold").toString().toInt();
    float n = data.getEchoData().length();

    //scanningOutput(data, cast);


    scene.clear();
    int height = ui->graphicsView->height()-1;
    float faktor = (height / range);
    if(range >= 20.0){
        faktor = faktor * 10;
    }

    //    scene.addLine(-256,280,256,280,QPen(QColor("red")))->setZValue(10);


    if (ui->updateView->isChecked())
    {
        for(int i = 1; i < range+1; i++)
        {

            scene.addLine(0,(i*faktor),ui->graphicsView->width(),(i*faktor),QPen(QColor(255,218,185,255)))->setZValue(10);
        }

        QLinearGradient gi(0,0,0,279);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];

            if(i != avgTemp){

                // Just in case da ist mal ein negativer Wert drin ;)
                if(b < 0){
                    b = b*(-1);
                }
                if(ui->filter->isChecked()){
                    if(b>threshold){
                        gi.setColorAt(1.0*i/n,QColor(255-2*b,255-2*b,255-2*b));
                    } else {
                        b = 0;
                        gi.setColorAt(1.0*i/n,QColor(255-2*b,255-2*b,255-2*b));
                    }
                } else {
                    gi.setColorAt(1.0*i/n,QColor(255-2*b,255-2*b,255-2*b));
                }
            } else {
                //qDebug("avg Print");
                gi.setColorAt(1.0*i/n,QColor(255,000,000));
            }
        }

        dataQueue.append(gi);
        dataQueue.pop_front();

        for(int i =0; i<ui->graphicsView->width()/1; i++)
        {
            QGraphicsRectItem *rit = scene.addRect((i*1),1,(1)*1,274,(Qt::NoPen));
            rit->setBrush(QBrush(dataQueue[i]));
        }
    }

}


void EchoSounderForm::updateEchoUi(float avgDistance){
    //qDebug("Array Pos");
    avgTemp = avgDistance*einheit;
    //qDebug()<< avgTemp;
    ui->avgDistance->setText(QString::number(avgDistance));
    //qDebug(a);
}

void EchoSounderForm::on_save_clicked()
{
    echo->setSettingsValue("serialPort",ui->serialPort->text());
    ui->port->setText(echo->getSettingsValue("serialPort").toString());
    echo->setSettingsValue("range",ui->range->text());
    ui->echoRange->setText(echo->getSettingsValue("range").toString());
    echo->setSettingsValue("gain", ui->gain->text());
    ui->gainText->setText(echo->getSettingsValue("gain").toString());
    echo->setSettingsValue("scanTimer", ui->timerInput->text());
    ui->timer->setText(echo->getSettingsValue("scanTimer").toString());
    echo->setSettingsValue("calcFactor",ui->factorInput->text());

    echo->setSettingsValue("threshold", ui->thresholdInput->text());
    ui->thresholdInput->setText(echo->getSettingsValue("threshold").toString());
    ui->thresholdText->setText(echo->getSettingsValue("threshold").toString());
    echo->setSettingsValue("averageWindow", ui->averageWindowInput->text());
    QTimer::singleShot(0,echo,SLOT(reset()));

    // Hier fehlt ggf. noch ein löschen der view, wenn die range geändert wird.

}


void EchoSounderForm::on_applyButton_clicked()
{
    echo->setSettingsValue("readFromFile", ui->sourceFile->isChecked());
    echo->setSettingsValue("filename", ui->fileName->text());
    echo->setSettingsValue("enableRecording", ui->enableRecording->isChecked());
    echo->setSettingsValue("formatCSV", ui->formatCSV->isChecked());
    QTimer::singleShot(0,echo,SLOT(reset()));
}

void EchoSounderForm::on_selFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Echo Recording", ui->fileName->text(), "Recording (*.852)");

    if (fileName.length()>0)
        ui->fileName->setText(fileName);
}

