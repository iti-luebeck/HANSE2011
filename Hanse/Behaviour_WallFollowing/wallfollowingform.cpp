#include "wallfollowingform.h"
#include "ui_wallfollowingform.h"
#include <QFileDialog>
#include <Framework/dataloghelper.h>


WallFollowingForm::WallFollowingForm(QWidget *parent, Behaviour_WallFollowing *wallfollowing) :
        QWidget(parent),
        ui(new Ui::WallFollowingForm)
{
    this->wallfollow = wallfollowing;

    QObject::connect(this,SIGNAL(startBehaviour()),wallfollow,SLOT(startBehaviour()));
    QObject::connect(this,SIGNAL(stopBehaviour()),wallfollow,SLOT(stop()));

    connect(wallfollow,SIGNAL(newWallUiData(const EchoReturnData, float)),this,SLOT(updateWallUi(const EchoReturnData, float)));
    connect(wallfollow,SIGNAL(updateGUI(QString)),this,SLOT(updateWallCase(QString)));

    connect(wallfollow,SIGNAL(updateUi()),this,SLOT(updateUiView()));

    ui->setupUi(this);

    this->ui->graphicsView->setScene(&scene);

    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("white"));
    gi.setColorAt(1,QColor("white"));
    for(int i =0; i<ui->graphicsView->width()/1; i++)
    {
        dataQueue.append(gi);
    }
    this->ui->port->setText(wallfollow->getSettingsValue("serialPort").toString());
    this->ui->echoRange->setText(wallfollow->getSettingsValue("range").toString());
    this->ui->forwardInput->setText(wallfollow->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(wallfollow->getSettingsValue("angularSpeed").toString());
    this->ui->distanceInput->setText(wallfollow->getSettingsValue("desiredDistance").toString());
    this->ui->corridorInput->setText(wallfollow->getSettingsValue("corridorWidth").toString());
    this->ui->updateView->setChecked(wallfollow->getSettingsValue("updateView").toBool());
    this->ui->pInput->setText(wallfollow->getSettingsValue("p").toString());
    this->ui->useP->setChecked(wallfollow->getSettingsValue("useP").toBool());
    this->ui->allowedInput->setText(wallfollow->getSettingsValue("allowedDist").toString());
    this->ui->driveInput->setText(wallfollow->getSettingsValue("driveTime").toString());
    this->ui->useStartAdjust->setChecked(wallfollow->getSettingsValue("useStartAdjust").toBool());
}

WallFollowingForm::~WallFollowingForm()
{
    delete ui;
}

void WallFollowingForm::changeEvent(QEvent *e)
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

void WallFollowingForm::on_startButton_clicked()
{
    wallfollow->setSettingsValue("desiredDistance", ui->distanceInput->text());
    wallfollow->setSettingsValue("forwardSpeed", ui->forwardInput->text());
    wallfollow->setSettingsValue("angularSpeed", ui->angularInput->text());
    wallfollow->setSettingsValue("corridorWidth", ui->corridorInput->text());
    wallfollow->setSettingsValue("useP", ui->useP->isChecked());
    wallfollow->setSettingsValue("p", ui->pInput->text().toFloat());
    wallfollow->setSettingsValue("updateView", this->ui->updateView->isChecked());
    wallfollow->setSettingsValue("allowedDist", ui->allowedInput->text());
    wallfollow->setSettingsValue("driveTime", ui->driveInput->text());
    wallfollow->setSettingsValue("useStartAdjust", this->ui->useStartAdjust->isChecked());
    QTimer::singleShot(0, wallfollow, SLOT(updateFromSettings()));
    emit startBehaviour();

}

void WallFollowingForm::on_applyButton_clicked(){
    wallfollow->setSettingsValue("desiredDistance", ui->distanceInput->text());
    wallfollow->setSettingsValue("forwardSpeed", ui->forwardInput->text());
    wallfollow->setSettingsValue("angularSpeed", ui->angularInput->text());
    wallfollow->setSettingsValue("corridorWidth", ui->corridorInput->text());
    wallfollow->setSettingsValue("useP", ui->useP->isChecked());
    wallfollow->setSettingsValue("p", ui->pInput->text().toFloat());
    wallfollow->setSettingsValue("allowedDist", ui->allowedInput->text());
    wallfollow->setSettingsValue("driveTime", ui->driveInput->text());
    wallfollow->setSettingsValue("updateView", this->ui->updateView->isChecked());
    wallfollow->setSettingsValue("useStartAdjust", this->ui->useStartAdjust->isChecked());
    QTimer::singleShot(0, wallfollow, SLOT(updateFromSettings()));
}

void WallFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}

void WallFollowingForm::updateWallUi(const EchoReturnData data, float dist)
{
    float einheit =  data.getEchoData().length()/data.getRange();
    int avgTemp = dist*einheit;
    float range = data.getRange();

    float n = data.getEchoData().length();
    scene.clear();
    int height = ui->graphicsView->height()-1;
    float faktor = (height / range);
    if(range >= 20.0){
        faktor = faktor * 10;
    }
    //    scene.addLine(-256,280,256,280,QPen(QColor("red")))->setZValue(10);


    if (ui->updateView->isChecked())
    {
        //qDebug("updateWallUi");
        for(int i = 1; i < range+1; i++)
        {

            scene.addLine(0,(i*faktor),ui->graphicsView->width(),(i*faktor),QPen(QColor(255,218,185,255)))->setZValue(10);
        }

        QLinearGradient gi(0,0,0,279);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            // Just in case da ist mal ein negativer Wert drin ;)
            if(b < 0){
                b = b*(-1);
            }
            if(i != avgTemp){
                gi.setColorAt(1.0*i/n,QColor(255-2*b,255-2*b,255-2*b));
            } else {
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
    QString result;
    QTextStream(&result) << dist;
    this->ui->avgDistance->setText(result);
    QString result2;
    QTextStream(&result2) << range;
    this->ui->r2->setText(result2);

}

void WallFollowingForm::updateWallCase(QString caseW){
    ui->currentCase->setText(caseW);
}

void WallFollowingForm::updateUiView(){
    this->ui->port->setText(wallfollow->getSettingsValue("serialPort").toString());
    this->ui->echoRange->setText(wallfollow->getSettingsValue("range").toString());
    this->ui->forwardInput->setText(wallfollow->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(wallfollow->getSettingsValue("angularSpeed").toString());
    this->ui->distanceInput->setText(wallfollow->getSettingsValue("desiredDistance").toString());
    this->ui->corridorInput->setText(wallfollow->getSettingsValue("corridorWidth").toString());
    this->ui->updateView->setChecked(wallfollow->getSettingsValue("updateView").toBool());
    this->ui->useP->setChecked(wallfollow->getSettingsValue("useP").toBool());
    this->ui->pInput->setText(wallfollow->getSettingsValue("p").toString());
    this->ui->allowedInput->setText(wallfollow->getSettingsValue("allowedDist").toString());
    this->ui->driveInput->setText(wallfollow->getSettingsValue("driveTime").toString());
    this->ui->useStartAdjust->setChecked(wallfollow->getSettingsValue("useStartAdjust").toBool());
}

