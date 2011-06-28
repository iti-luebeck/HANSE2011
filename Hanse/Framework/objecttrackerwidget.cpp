#include "objecttrackerwidget.h"
#include "ui_objecttrackerwidget.h"

ObjectTrackerWidget::ObjectTrackerWidget(ObjectTracker *tracker, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ObjectTrackerWidget)
{
    this->tracker = tracker;
    ui->setupUi(this);
    loadTrackerData();
    QObject::connect(tracker, SIGNAL(updateComplete()), this, SLOT(trackerUpdated()));

    ui->enableOutputBox->setChecked(false);
}

ObjectTrackerWidget::~ObjectTrackerWidget()
{
    delete ui;
}

void ObjectTrackerWidget::on_applyButton_clicked()
{
    storeTrackerData();
}

void ObjectTrackerWidget::loadTrackerData()
{
    QString colorSpace = tracker->getColorSpace();
    if (colorSpace == "rgb") {
        ui->rgbButton->setChecked(true);
        ui->hsvButton->setChecked(false);
    } else {
        ui->rgbButton->setChecked(false);
        ui->hsvButton->setChecked(true);
    }

    int channel = tracker->getChannel();
    if (channel == 0) {
        ui->oneButton->setChecked(true);
        ui->twoButton->setChecked(false);
        ui->threeButton->setChecked(false);
    } else if (channel == 1) {
        ui->oneButton->setChecked(false);
        ui->twoButton->setChecked(true);
        ui->threeButton->setChecked(false);
    } else {
        ui->oneButton->setChecked(false);
        ui->twoButton->setChecked(false);
        ui->threeButton->setChecked(true);
    }

    ui->invertedCheckBox->setChecked(tracker->isInverted());
    ui->automaticCheckBox->setChecked(tracker->isAutomaticThreshold());
    ui->thresholdSlider->setValue(tracker->getThreshold());
}

void ObjectTrackerWidget::storeTrackerData()
{
    QString colorSpace = "rgb";
    if (ui->hsvButton->isChecked()) {
        colorSpace = "hsv";
    }

    int channel = 0;
    if (ui->twoButton->isChecked()) {
        channel = 1;
    } else if (ui->threeButton->isChecked()) {
        channel = 2;
    }

    bool inverted = ui->invertedCheckBox->isChecked();
    bool automatic = ui->automaticCheckBox->isChecked();
    double thres = ui->thresholdSlider->value();

    tracker->setAutomaticThreshold(automatic);
    tracker->setChannel(channel);
    tracker->setColorSpace(colorSpace);
    tracker->setInverted(inverted);
    tracker->setThreshold(thres);
}

void ObjectTrackerWidget::trackerUpdated()
{
    if (ui->enableOutputBox->isChecked()) {
        cv::Mat frame;
        cv::Mat gray;
        cv::Mat binary;
        tracker->grabFrame(frame);
        tracker->grabGray(gray);
        tracker->grabBinary(binary);

        if (!frame.empty()) {
            QImage frameImage((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
            ui->originalLabel->setPixmap(QPixmap::fromImage(frameImage));
        }

        if (!gray.empty()) {
            QImage grayImage((unsigned char*)gray.data, gray.cols, gray.rows, QImage::Format_RGB888);
            ui->grayLabel->setPixmap(QPixmap::fromImage(grayImage));
        }

        if (!binary.empty()) {
            QImage binaryImage((unsigned char*)binary.data, binary.cols, binary.rows, QImage::Format_RGB888);
            ui->binaryLabel->setPixmap(QPixmap::fromImage(binaryImage));
        }
    }
}
