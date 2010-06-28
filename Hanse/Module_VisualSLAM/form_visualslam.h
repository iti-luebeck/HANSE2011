#ifndef FORM_VISUALSLAM_H
#define FORM_VISUALSLAM_H

#include <Module_VisualSLAM/module_visualslam.h>

#include <QWidget>

class Module_VisualSLAM;

namespace Ui {
    class Form_VisualSLAM;
}

class Form_VisualSLAM : public QWidget {
    Q_OBJECT
public:
    Form_VisualSLAM( Module_VisualSLAM *visualSlam, QWidget *parent = 0 );
    ~Form_VisualSLAM();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_VisualSLAM *ui;
    Module_VisualSLAM *visualSlam;

private slots:
    void on_checkBox_clicked();
    void on_applyButton_clicked();
    void on_resetButton_clicked();
    void on_stopButton_clicked();
    void on_startButton_clicked();

signals:
    void settingsChanged( double v_observation, double v_translation, double v_rotation );
};

#endif // FORM_VISUALSLAM_H
