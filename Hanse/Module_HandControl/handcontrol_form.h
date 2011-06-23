#ifndef HANDCONTROL_FORM_H
#define HANDCONTROL_FORM_H

#include <QWidget>
#include "module_handcontrol.h"
#include "server.h"

namespace Ui {
    class HandControl_Form;
}

class HandControl_Form : public QWidget {
    Q_OBJECT
public:
    HandControl_Form(Module_HandControl *module, QWidget *parent = 0);
    ~HandControl_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::HandControl_Form *ui;
    Module_HandControl *module;

    QTimer forTimer;
    QTimer angTimer;

    QTimer testTimer;

    QAction *forward;
    QAction *backward;
    QAction *left;
    QAction *right;
    QAction *up;
    QAction *down;

    float maxForwardSpeed;
    float maxAngularSpeed;
    float upDownSpeed;
    float stepsize;

    float fwd;
    float ang;
    float dep;

private slots:
    void on_resetDepthButton_clicked();
    void on_downButton_clicked();
    void on_upButton_clicked();
    void on_rightButton_clicked();
    void on_backwardButton_clicked();
    void on_leftButton_clicked();
    void on_forwardButton_clicked();
    void on_save_clicked();
    void connectionStatusChanged();
//    void dataChanged(RobotModule* m);
    void resetForSpeeds();
    void resetAngSpeeds();

    void setSpeeds();

signals:
    void updateControls(int forwardSpeed, int angularSpeed, int speedUpDown);
};

#endif // HANDCONTROL_FORM_H
