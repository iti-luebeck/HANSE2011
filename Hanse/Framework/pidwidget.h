#ifndef PIDWIDGET_H
#define PIDWIDGET_H

#include <QWidget>
#include <Framework/pidcontroller.h>
#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>

namespace Ui {
    class PIDWidget;
}

class PIDWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PIDWidget(PIDController *pid, QWidget *parent = 0);
    ~PIDWidget();

private:
    Ui::PIDWidget *ui;

    QwtPlot *plot;
    QwtPlotCurve *curveSetpoint;
    QwtPlotCurve *curveActual;
    QwtPlotCurve *curveU;

    QMap<QDateTime,double> historyActual;
    QMap<QDateTime,double> historySetpoint;
    QMap<QDateTime,double> historyU;

public slots:
    void newPIDData(double setpoint, double actual, double u);

signals:
    void setValues(double Kp, double Ti, double Td, double offset, double min, double max, double minHysteresis, double maxHysteresis, double minUpdateTime);
};

#endif // PIDWIDGET_H
