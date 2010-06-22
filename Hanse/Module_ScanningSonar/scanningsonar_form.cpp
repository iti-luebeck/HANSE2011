#include "scanningsonar_form.h"
#include "ui_scanningsonar_form.h"
#include "sonarreturndata.h"
#include <Framework/dataloghelper.h>

ScanningSonarForm::ScanningSonarForm(Module_ScanningSonar* sonar, QWidget *parent) :
        QWidget(parent), scene(),
    ui(new Ui::ScanningSonarForm)
{
    ui->setupUi(this);
    this->sonar = sonar;

    oldHeading = NAN;
    oldStepSize = 0;

    logger = Log4Qt::Logger::logger("ScanningSonarForm");

    this->ui->graphicsView->setScene(&scene);
    scanLine = scene.addLine(0,0,0,50, QPen(QColor("red")));
    scanLine->setZValue(20);

    scene.addLine(-50,0,50,0,QPen(QColor("white")))->setZValue(10);
    scene.addLine(0,-50,0,50,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-50,-50,100,100,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-40,-40,80,80,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-30,-30,60,60,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-20,-20,40,40,QPen(QColor("white")))->setZValue(10);
    scene.addEllipse(-10,-10,20,20,QPen(QColor("white")))->setZValue(10);

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), this, SLOT(updateSonarView(SonarReturnData)));

    ui->serialPort->setText(sonar->getSettings().value("serialPort").toString());
    ui->frequency->setText(sonar->getSettings().value("frequency").toString());
    ui->gain->setValue(sonar->getSettings().value("gain").toInt());
    ui->pulseLength->setValue(sonar->getSettings().value("pulseLength").toInt());
    ui->range->setValue(sonar->getSettings().value("range").toInt());
    ui->sectorWidth->setValue(sonar->getSettings().value("sectorWidth").toInt());
    ui->stepSize->setCurrentIndex(sonar->getSettings().value("stepSize").toInt()-1);
    ui->switchDelay->setText(sonar->getSettings().value("switchDelay").toString());
    ui->trainAngle->setText(sonar->getSettings().value("trainAngle").toString());
    ui->dataPoints->setText(sonar->getSettings().value("dataPoints").toString());

    ui->sourceFile->setChecked(sonar->getSettings().value("readFromFile").toBool());
    ui->sourceSerial->setChecked(!sonar->getSettings().value("readFromFile").toBool());
    ui->fileName->setText(sonar->getSettings().value("filename").toString());
    ui->enableRecording->setChecked(sonar->getSettings().value("enableRecording").toBool());
    ui->fileReaderDelay->setValue(sonar->getSettings().value("fileReaderDelay").toInt());
    ui->formatCSV->setChecked(sonar->getSettings().value("formatCSV").toBool());
    ui->format852->setChecked(!sonar->getSettings().value("formatCSV").toBool());
    ui->startTime->setDateTime(sonar->getSettings().value("startTime").toDateTime());

    ui->recorderFilename->setText(DataLogHelper::getLogDir()+"sonarlog.XXX");
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

    float range = data.getRange();

    if (oldStepSize != data.switchCommand.stepSize) {
        oldStepSize = data.switchCommand.stepSize;
        foreach (QGraphicsPolygonItem* o, queue) {
            delete o;
        }
        queue.clear();
    }

    ui->time->setDateTime(data.switchCommand.time);
    ui->heading->setText(QString::number(data.getHeadPosition()));
    ui->gain_2->setText(QString::number(data.switchCommand.startGain));
    ui->range_2->setText(QString::number(data.getRange()));

    // TODO: if any of the parameters have changed; reset the scene

    float newHeading = data.getHeadPosition();

    if (ui->checkBox->isChecked() && !isnan(oldHeading)
        && (fabs(newHeading - oldHeading)<20 || fabs(newHeading - oldHeading)>340)) {

        QPolygonF polygon;

        QPointF endPoint1 = QTransform().rotate(oldHeading).map(QPointF(range,0));
        QPointF endPoint2 = QTransform().rotate(newHeading).map(QPointF(range,0));

        polygon << QPointF(0,0) << endPoint1 << endPoint2;
        QGraphicsPolygonItem *it = scene.addPolygon(polygon,QPen(Qt::NoPen));
        queue.append(it);

        scanLine->setRotation(newHeading-90);

        QLinearGradient g(QPointF(0, 0), endPoint2);
        for (int i = 0; i < n; i++) {
            char b = data.getEchoData()[i];
            g.setColorAt(1.0*i/n,QColor(0,2*b,0));
        }
        it->setBrush(QBrush(g));

        // this should ensure that a full circle is conserved even at highest resolution
        // it may result in overlay, but this doesn't matter since newer items will always
        // be drawn on top of older ones.
        // 480: don't ask, it just works :)
        while (queue.size()>480/(oldStepSize*3+3)) {
            delete queue.takeFirst();
        }

    }

    oldHeading = newHeading;
}

void ScanningSonarForm::on_save_clicked()
{
    sonar->getSettings().setValue("serialPort", ui->serialPort->text());
    sonar->getSettings().setValue("frequency", ui->frequency->text().toInt());
    sonar->getSettings().setValue("gain", ui->gain->value());
    sonar->getSettings().setValue("pulseLength", ui->pulseLength->value());
    sonar->getSettings().setValue("range", ui->range->value());
    sonar->getSettings().setValue("sectorWidth", ui->sectorWidth->value());
    sonar->getSettings().setValue("stepSize", ui->stepSize->currentIndex()+1);
    sonar->getSettings().setValue("switchDelay", ui->switchDelay->text().toInt());
    sonar->getSettings().setValue("trainAngle", ui->trainAngle->text().toInt());
    sonar->getSettings().setValue("dataPoints", ui->dataPoints->text().toInt());
    sonar->reset();

    // the scan resolution may have changed, clear the graphics scene
    foreach(QGraphicsItem* g, queue) {
        delete queue.takeFirst();
    }
}

void ScanningSonarForm::on_fileCfgApply_clicked()
{
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

void ScanningSonarForm::on_selFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Sonar Recording", ui->fileName->text(), "Recording (*.852)");

    if (fileName.length()>0)
        ui->fileName->setText(fileName);
}
