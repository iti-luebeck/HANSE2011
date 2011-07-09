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

private:
    Ui::Form_Webcams *ui;
    Module_Webcams *cams;
    QTimer captureTimer;

    cv::Mat frontFrame;
    cv::Mat bottomFrame;
    int frontCount;
    int bottomCount;

private slots:
    void on_frontConnectCheckBox_clicked();
    void on_bottomConnectCheckBox_clicked();
    void on_updateListButton_clicked();

public slots:
    void newFrontImage(cv::Mat front);
    void newBottomImage(cv::Mat bottom);

signals:
    void changedSettings();
    void captureTurnedOn();
    void captureTurnedOff();
    void showSettings( int camNr );
};

#endif // FORM_WEBCAMS_H
