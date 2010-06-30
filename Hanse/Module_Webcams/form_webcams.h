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
    Ui::Form_Webcams *ui;
    Module_Webcams *cams;
    IplImage *leftFrame;
    IplImage *rightFrame;
    IplImage *bottomFrame;

private slots:
    void on_refreshButton_clicked();
    void on_applyButtn_clicked();

signals:
    void changedSettings();
};

#endif // FORM_WEBCAMS_H
