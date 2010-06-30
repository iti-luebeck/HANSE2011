#ifndef FORM_WEBCAMS_H
#define FORM_WEBCAMS_H

#include <QWidget>
#include <opencv/cxcore.h>

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
    IplImage *leftFrame;
    IplImage *rightFrame;
    IplImage *bottomFrame;

private slots:
    void on_bottomSettingsButton_clicked();
    void on_rightSettingsButton_clicked();
    void on_leftSettingsButton_clicked();
    void on_checkBox_clicked();
    void on_updateListButton_clicked();
    void on_refreshButton_clicked();
    void on_applyButtn_clicked();

signals:
    void changedSettings();
    void captureTurnedOn();
    void captureTurnedOff();
    void showSettings( int camNr );
};

#endif // FORM_WEBCAMS_H
