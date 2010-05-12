#ifndef IMU_FORM_H
#define IMU_FORM_H

#include <QWidget>
#include "module_imu.h"

namespace Ui {
    class IMU_Form;
}

class IMU_Form : public QWidget {
    Q_OBJECT
public:
    IMU_Form(Module_IMU *module, QWidget *parent = 0);
    ~IMU_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::IMU_Form *ui;
};

#endif // IMU_FORM_H
