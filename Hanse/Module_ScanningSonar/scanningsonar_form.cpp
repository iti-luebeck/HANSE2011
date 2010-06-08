#include "scanningsonar_form.h"
#include "ui_scanningsonar_form.h"
#include "sonarreturndata.h"

ScanningSonarForm::ScanningSonarForm(Module_ScanningSonar* sonar, QWidget *parent) :
        QWidget(parent), scene(-200,-200,500,500),
    ui(new Ui::ScanningSonarForm)
{
    ui->setupUi(this);
    this->sonar = sonar;

    oldHeading = -1000;

    this->ui->graphicsView->setScene(&scene);
    scanLine = scene.addLine(0,0,0,500, QPen(QColor("red")));
    scanLine->setZValue(1);

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

    ui->sourceFile->setChecked(sonar->getSettings().value("readFromFile").toBool());
    ui->sourceSerial->setChecked(!sonar->getSettings().value("readFromFile").toBool());
    ui->fileName->setText(sonar->getSettings().value("filename").toString());
    ui->recorderFilename->setText(sonar->getSettings().value("recorderFilename").toString());
    ui->enableRecording->setChecked(sonar->getSettings().value("enableRecording").toBool());
    ui->fileReaderDelay->setValue(sonar->getSettings().value("fileReaderDelay").toInt());
    ui->formatCSV->setChecked(sonar->getSettings().value("formatCSV").toBool());
    ui->format852->setChecked(!sonar->getSettings().value("formatCSV").toBool());
    ui->startTime->setDateTime(sonar->getSettings().value("startTime").toDateTime());
}

ScanningSonarForm::~ScanningSonarForm()
{
    delete ui;
}

void ScanningSonarForm::changeEvent(QEvent *e)
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

void ScanningSonarForm::updateSonarView(const SonarReturnData data)
{
    float n = data.getEchoData().length();

    ui->time->setDateTime(data.dateTime);
    ui->heading->setText(QString::number(data.getHeadPosition()));
    ui->gain_2->setText(QString::number(data.startGain));
    ui->range_2->setText(QString::number(data.getRange()));

    if (!ui->checkBox->isChecked())
        return;

    if (oldHeading>-1000) {

        if (!map.contains(data.getHeadPosition())) {
            QPolygonF polygon;
            float endX = n;

            float diff = data.getHeadPosition() - oldHeading;
            if (diff>20)
                diff -= 360;

            float endY = tan(diff/180*M_PI)*endX;

//            QTransform t;
//            t.rotate(oldHeading);
//            QPointF endPoint1 = t.map(QPointF(n,0));
//            t.reset();
//            t.rotate(data.getHeadPosition());
//            QPointF endPoint2 = t.map(QPointF(n,0));

//            polygon << QPointF(0,0) << endPoint1 << endPoint2;
            polygon << QPointF(0,0) << QPointF(endX,0)<< QPointF(endX,endY);
            QGraphicsPolygonItem *it = scene.addPolygon(polygon,QPen(Qt::NoPen));
            map[data.getHeadPosition()] = it;
            it->setRotation(data.getHeadPosition()+90);
        }

        scanLine->setRotation(data.getHeadPosition());

        QGraphicsPolygonItem *it = map[data.getHeadPosition()];

        QLinearGradient g(QPointF(0, 0), QPointF(n,0));
        for (int i = 0; i < data.getEchoData().length(); ++i) {
            char b = data.getEchoData()[i];
            g.setColorAt(i/n,QColor(0,b,0));
        }
        it->setBrush(QBrush(g));

    }

    oldHeading = data.getHeadPosition();
}

void ScanningSonarForm::on_save_clicked()
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

void ScanningSonarForm::on_fileCfgApply_clicked()
{
    sonar->getSettings().setValue("recorderFilename", ui->recorderFilename->text());
    sonar->getSettings().setValue("readFromFile", ui->sourceFile->isChecked());
    sonar->getSettings().setValue("filename", ui->fileName->text());
    sonar->getSettings().setValue("fileReaderDelay", ui->fileReaderDelay->value());
    sonar->getSettings().setValue("enableRecording", ui->enableRecording->isChecked());
    sonar->getSettings().setValue("formatCSV", ui->formatCSV->isChecked());
    sonar->getSettings().setValue("startTime", ui->startTime->dateTime());

    sonar->reset();
}

void ScanningSonarForm::on_fileReaderDelay_valueChanged(int )
{
    sonar->getSettings().setValue("fileReaderDelay", ui->fileReaderDelay->value());
}
