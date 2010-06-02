#include "form.h"
#include "ui_form.h"
#include "sonarreturndata.h"

Form::Form(Module_ScanningSonar* sonar, QWidget *parent) :
        QWidget(parent), scene(-500,-500,1000,1000),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->sonar = sonar;

    this->ui->graphicsView->setScene(&scene);
    scanLine = scene.addLine(0,0,0,500, QPen(QColor("red")));

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), this, SLOT(updateSonarView(SonarReturnData)));

    ui->serialPort->setText(sonar->getSettings().value("serialPort").toString());
    ui->frequency->setText(sonar->getSettings().value("frequency").toString());
    ui->gain->setValue(sonar->getSettings().value("gain").toInt());
    ui->pulseLength->setValue(sonar->getSettings().value("pulseLength").toInt());
    ui->range->setValue(sonar->getSettings().value("range").toInt());
    ui->sectorWidth->setValue(sonar->getSettings().value("sectorWidth").toInt());
    ui->stepSize->setValue(sonar->getSettings().value("stepSize").toInt());
    ui->switchDelay->setText(sonar->getSettings().value("switchDelay").toString());
    ui->trainAngle->setText(sonar->getSettings().value("trainAngle").toString());
    ui->dataPoints->setText(sonar->getSettings().value("dataPoints").toString());
    ui->readFileCheckbox->setChecked(sonar->getSettings().value("readFromFile").toBool());
    ui->fileName->setText(sonar->getSettings().value("filename").toString());
    ui->recorderFilename->setText(sonar->getSettings().value("recorderFilename").toString());
    ui->enableRecording->setChecked(sonar->getSettings().value("enableRecording").toBool());
    ui->fileReaderDelay->setValue(sonar->getSettings().value("fileReaderDelay").toInt());

}

Form::~Form()
{
    delete ui;
}

void Form::changeEvent(QEvent *e)
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

void Form::updateSonarView(SonarReturnData data)
{
    QGraphicsItem* it = scene.addLine(0,0,0,0);

    if (map.contains(data.getHeadPosition())) {
        QGraphicsItem* old = map[data.getHeadPosition()];
        map.remove(data.getHeadPosition());
        scene.removeItem(old);
        delete old;
    }

    map[data.getHeadPosition()] = it;

    int th = 50;
    int r = data.getRange();
    for (int i = 0; i < data.getEchoData().length(); ++i) {
        char b = data.getEchoData()[i];
        QPolygonF polygon;
        float startX = i-0.5;
        float endX = i+0.5;
        float startY = tan(0.052/2)*startX;
        float endY = tan(0.052/2)*endX;
        polygon << QPointF(startX,startY) << QPointF(endX,endY)<< QPointF(endX,-endY)<< QPointF(startX,-startY)<< QPointF(startX,startY);
        QGraphicsPolygonItem *segment = new QGraphicsPolygonItem(polygon,it);
        segment->setPen(QPen(QColor(0,b,0)));
    }

    scanLine->setRotation(data.getHeadPosition());
    it->setRotation(data.getHeadPosition()+90);
}

void Form::on_save_clicked()
{
    sonar->getSettings().setValue("serialPort", ui->serialPort->text());
    sonar->getSettings().setValue("frequency", ui->frequency->text().toInt());
    sonar->getSettings().setValue("gain", ui->gain->value());
    sonar->getSettings().setValue("pulseLength", ui->pulseLength->value());
    sonar->getSettings().setValue("range", ui->range->value());
    sonar->getSettings().setValue("sectorWidth", ui->sectorWidth->value());
    sonar->getSettings().setValue("stepSize", ui->stepSize->value());
    sonar->getSettings().setValue("switchDelay", ui->switchDelay->text().toInt());
    sonar->getSettings().setValue("trainAngle", ui->trainAngle->text().toInt());
    sonar->getSettings().setValue("dataPoints", ui->dataPoints->text().toInt());
    sonar->reset();

    // the scan resolution may have changed, clear the graphics scene
    foreach(QGraphicsItem* g, map.values()) {
        scene.removeItem(g);
        delete g;
    }
    map.clear();
}

void Form::on_fileCfgApply_clicked()
{
    sonar->getSettings().setValue("recorderFilename", ui->recorderFilename->text());
    sonar->getSettings().setValue("readFromFile", ui->readFileCheckbox->isChecked());
    sonar->getSettings().setValue("filename", ui->fileName->text());
    sonar->getSettings().setValue("fileReaderDelay", ui->fileReaderDelay->value());
    sonar->getSettings().setValue("enableRecording", ui->enableRecording->isChecked());
    sonar->reset();
}
