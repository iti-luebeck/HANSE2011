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
    Module_IMU *module;
    QTimer timer;

private slots:
    void on_calibPrecise_clicked();
    void on_calibNull_clicked();
    void on_save_clicked();
    void updateBiasFields();
signals:
    void doPrecisionCalib();
    void doNullCalib();
    void reset();
    void newDataValue(QString key, const QVariant data);
    void newSettingsValue(QString key, const QVariant value);
};

#endif // IMU_FORM_H
