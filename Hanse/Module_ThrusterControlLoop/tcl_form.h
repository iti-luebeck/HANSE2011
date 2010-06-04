#ifndef TCL_FORM_H
#define TCL_FORM_H

#include <QWidget>
#include "module_thrustercontrolloop.h"

#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>

namespace Ui {
    class TCL_Form;
}

class TCL_Form : public QWidget {
    Q_OBJECT
public:
    TCL_Form(Module_ThrusterControlLoop *module, QWidget *parent = 0);
    ~TCL_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TCL_Form *ui;
    Module_ThrusterControlLoop *module;
    QwtPlot *plot;
    QwtPlotCurve *curveSoll;
    QwtPlotCurve *curveIst;
    QwtPlotCurve *curveThruster;

private slots:
    void on_save_clicked();
    void dataChanged(RobotModule* mod);
};

#endif // TCL_FORM_H
