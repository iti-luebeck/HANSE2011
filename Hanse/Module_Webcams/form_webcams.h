#ifndef FORM_WEBCAMS_H
#define FORM_WEBCAMS_H

#include <QWidget>
#include <opencv/cxcore.h>
#include <QTimer>

class Module_Webcams;

namespace Ui {
    class Form_Webcams;
}

class Form_Webcams : public QWidget {
    Q_OBJECT
public:
    Form_Webcams( Module_Webcams *cams, QWidget *parent = 0 );
    ~Form_Webcams();

protected:
    void changeEvent(QEvent *e);

private:
    void refreshLists();
    void refreshFrames();

private:
    Ui::Form_Webcams *ui;
    Module_Webcams *cams;
    QTimer captureTimer;

    cv::Mat leftFrame;
    cv::Mat rightFrame;
    cv::Mat bottomFrame;
    int count;

private slots:
    void on_bottomFrameRateSlider_sliderMoved(int position);
    void on_rightFrameRateSlider_sliderMoved(int position);
    void on_leftFrameRateSlider_sliderMoved(int position);
    void on_bottomConnectCheckBox_clicked();
    void on_rightConnectCheckBox_clicked();
    void on_leftConnectCheckBox_clicked();
    void on_checkBox_clicked();
    void on_updateListButton_clicked();
    void on_refreshButton_clicked();
    void on_applyButtn_clicked();

public slots:
    void captureWebcams();

signals:
    void changedSettings();
    void captureTurnedOn();
    void captureTurnedOff();
    void showSettings( int camNr );
};

#endif // FORM_WEBCAMS_H
