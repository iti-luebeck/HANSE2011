#include "scanningsonar_form.h"
#include "ui_scanningsonar_form.h"
#include "sonarreturndata.h"
#include <Framework/dataloghelper.h>

ScanningSonarForm::ScanningSonarForm(Module_ScanningSonar* sonar,QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ScanningSonarForm),
        scene()
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

    ui->serialPort->setText(sonar->getSettingsValue("serialPort").toString());
    ui->frequency->setText(sonar->getSettingsValue("frequency").toString());
    ui->gain->setValue(sonar->getSettingsValue("gain").toInt());
    ui->pulseLength->setValue(sonar->getSettingsValue("pulseLength").toInt());
    ui->range->setValue(sonar->getSettingsValue("range").toInt());
    ui->sectorWidth->setValue(sonar->getSettingsValue("sectorWidth").toInt());
    ui->stepSize->setCurrentIndex(sonar->getSettingsValue("stepSize").toInt()-1);
    ui->switchDelay->setText(sonar->getSettingsValue("switchDelay").toString());
    ui->trainAngle->setText(sonar->getSettingsValue("trainAngle").toString());
    ui->dataPoints->setText(sonar->getSettingsValue("dataPoints").toString());

    ui->sourceFile->setChecked(sonar->getSettingsValue("readFromFile").toBool());
    ui->sourceSerial->setChecked(!sonar->getSettingsValue("readFromFile").toBool());
    ui->fileName->setText(sonar->getSettingsValue("filename").toString());
    ui->enableRecording->setChecked(sonar->getSettingsValue("enableRecording").toBool());
    ui->fileReaderDelay->setValue(sonar->getSettingsValue("fileReaderDelay").toInt());
    ui->formatCSV->setChecked(sonar->getSettingsValue("formatCSV").toBool());
    ui->format852->setChecked(!sonar->getSettingsValue("formatCSV").toBool());
    ui->startTime->setDateTime(sonar->getSettingsValue("startTime").toDateTime());

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
    int bla = oldHeading;
    bool isnumber = (bla != 0);

    if(ui->checkBox->isChecked() && isnumber && (fabs(newHeading - oldHeading)<20 || fabs(newHeading - oldHeading)>340))
    {

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
    sonar->setSettingsValue("serialPort", ui->serialPort->text());
    sonar->setSettingsValue("frequency", ui->frequency->text().toInt());
    sonar->setSettingsValue("gain", ui->gain->value());
    sonar->setSettingsValue("pulseLength", ui->pulseLength->value());
    sonar->setSettingsValue("range", ui->range->value());
    sonar->setSettingsValue("sectorWidth", ui->sectorWidth->value());
    sonar->setSettingsValue("stepSize", ui->stepSize->currentIndex()+1);
    sonar->setSettingsValue("switchDelay", ui->switchDelay->text().toInt());
    sonar->setSettingsValue("trainAngle", ui->trainAngle->text().toInt());
    sonar->setSettingsValue("dataPoints", ui->dataPoints->text().toInt());
    sonar->reset();

    // the scan resolution may have changed, clear the graphics scene
    foreach(QGraphicsItem* g, queue) {
        delete queue.takeFirst();
    }
}

void ScanningSonarForm::on_fileCfgApply_clicked()
{
    sonar->setSettingsValue("readFromFile", ui->sourceFile->isChecked());
    sonar->setSettingsValue("filename", ui->fileName->text());
    sonar->setSettingsValue("fileReaderDelay", ui->fileReaderDelay->value());
    sonar->setSettingsValue("enableRecording", ui->enableRecording->isChecked());
    sonar->setSettingsValue("formatCSV", ui->formatCSV->isChecked());
    sonar->setSettingsValue("startTime", ui->startTime->dateTime());


    sonar->reset();
}

void ScanningSonarForm::on_fileReaderDelay_valueChanged(int )
{
    sonar->setSettingsValue("fileReaderDelay", ui->fileReaderDelay->value());
}

void ScanningSonarForm::on_selFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Sonar Recording", ui->fileName->text(), "Recording (*.852)");

    if (fileName.length()>0)
        ui->fileName->setText(fileName);
}
