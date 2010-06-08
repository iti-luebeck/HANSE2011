#ifndef FORM_SONARLOCALIZATION_H
#define FORM_SONARLOCALIZATION_H

#include <QWidget>

#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_symbol.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>

namespace Ui {
    class Form_SonarLocalization;
}

class Form_SonarLocalization : public QWidget {
    Q_OBJECT
public:
    Form_SonarLocalization(QWidget *parent, Module_SonarLocalization* m);
    ~Form_SonarLocalization();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_SonarLocalization *ui;
    QwtPlot *plot;
    QwtPlotCurve *curveRaw;
    QwtPlotCurve *curveFiltered;
    QwtPlotCurve *curveTH;
    QwtPlotCurve *curveVar;
    QwtPlotCurve *curveMean;
    QwtPlotMarker *curveK;

    Module_SonarLocalization* m;

private slots:
    void on_plotSelect_valueChanged(int );
};

#endif // FORM_SONARLOCALIZATION_H
