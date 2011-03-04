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

    ui->setupUi(this);

    this->ui->graphicsView->setScene(&scene);

    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("white"));
    gi.setColorAt(279,QColor("white"));
    for(int i =0; i<ui->graphicsView->width()/1; i++)
    {
        dataQueue.append(gi);
    }
    this->ui->port->setText(wallfollow->getSettingsValue("serialPort").toString());
    this->ui->echoRange->setText(wallfollow->getSettingsValue("range").toString());

    this->ui->updateView->setChecked(false);
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
    wallfollow->setSettingsValue("distanceInput", ui->distanceInput->text());

    //qDebug("startButton clicked");
    QTimer::singleShot(0,wallfollow,SLOT(reset()));
    //qDebug("startButton clicked2");
    emit startBehaviour();

}

void WallFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
    // QTimer::singleShot(0,&updateUI,SLOT(stop()));
}

void WallFollowingForm::updateWallUi(const EchoReturnData data, float dist)
{

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
        qDebug("updateWallUi");
        for(int i = 1; i < range+1; i++)
        {

            scene.addLine(0,(i*faktor),ui->graphicsView->width(),(i*faktor),QPen(QColor(200,83,83,255)))->setZValue(10);
        }

        QLinearGradient gi(0,0,0,279);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            gi.setColorAt(1.0*i/n,QColor(255-2*b,255-2*b,255-2*b));
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

}




