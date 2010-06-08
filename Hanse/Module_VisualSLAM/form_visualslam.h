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
    void setScene( QGraphicsScene *scene );

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_VisualSLAM *ui;
    Module_VisualSLAM *visualSlam;
    QMutex *sceneMutex;

public slots:
    void updateView();

private slots:
    void on_resetButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomOutButton_clicked();
    void on_stopButton_clicked();
    void on_startButton_clicked();
};

#endif // FORM_VISUALSLAM_H
