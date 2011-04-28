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
    QList<QGraphicsEllipseItem*> volatileItems;
    QList<QGraphicsEllipseItem*> particleItems;
    QGraphicsEllipseItem* currentPos;
    QGraphicsScene* scene;

    QList<SonarEchoData> sonarEchoDataList;
    QGraphicsScene scene2;
    QGraphicsScene sceneUnfiltered;
    QGraphicsScene sceneRaw;

    Ui::Form_SonarLocalization *ui;
    QwtPlot *plot;
    QwtPlotCurve *curveRaw;
    QwtPlotCurve *curveFiltered;
    QwtPlotCurve *curveTH;
    QwtPlotCurve *curveVar;
    QwtPlotCurve *curveMean;
    QwtPlotMarker *curveK;
    QwtPlotMarker *curveVarTH;

    Module_SonarLocalization* m;

    void setFields();
    void createPlot();
    void createMap();

    void updateSonarView();
    void updateSonarViewUnfiltered();
    void updateSonarViewRaw();

    public slots:
    void updateSonarViewList(QList<SonarEchoData> list);

private slots:
    void on_sltraining_clicked();
    void on_selSVM_clicked();
    void on_selSat_clicked();
    void on_selMap_clicked();
    void on_spinBox_valueChanged(int );
    void on_pushButton_clicked();
    void on_plotSelect_valueChanged(int );
    void newPositionEstimate(QVector3D estimate);

    void particleFilterStatus(bool);

};

#endif // FORM_SONARLOCALIZATION_H
