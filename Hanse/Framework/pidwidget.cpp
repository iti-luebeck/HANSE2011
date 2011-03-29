#include "pidwidget.h"
#include "ui_pidwidget.h"
#include <QtCore>
#include <QBoxLayout>
#include <qwt-qt4/qwt_legend.h>

#define PID_WIDGET_HISTORY  500

PIDWidget::PIDWidget(PIDController *pid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PIDWidget)
{
    ui->setupUi(this);

    QLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
    plot = new QwtPlot(this);
    l->addWidget(plot);
    this->setLayout(l);

    plot->setAxisScale(QwtPlot::yLeft,-1,6);
    plot->setAxisScale(QwtPlot::xBottom,0,300);

    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box|| QFrame::Sunken);
    plot->insertLegend(legend,QwtPlot::BottomLegend);

    curveActual = new QwtPlotCurve("ist");
    curveSetpoint = new QwtPlotCurve("soll");
    curveU = new QwtPlotCurve("control");

    curveActual->attach(plot);
    curveActual->setPen(QPen("red"));
    curveSetpoint->attach(plot);
    curveSetpoint->setPen(QPen("black"));
    curveU->attach(plot);
    curveU->setPen(QPen("blue"));

    plot->setTitle("depth control");
    plot->setAxisTitle(QwtPlot::xBottom,"time (s)");
    plot->setAxisTitle(QwtPlot::yLeft,"depth (m)");

    QObject::connect(pid, SIGNAL(newData(double,double,double)), this, SLOT(newPIDData(double,double,double)));
}

PIDWidget::~PIDWidget()
{
    delete ui;
}

void PIDWidget::newPIDData(double setpoint, double actual, double u)
{
    QDateTime now = QDateTime::currentDateTime();
    historyActual[now] = actual;
    historySetpoint[now] = setpoint;
    historyU[now] = u;

    QList<QDateTime> keys = historyActual.keys();

    QVector<double> axisTime;
    QVector<double> axisActual;
    QVector<double> axisSetpoint;
    QVector<double> axisU;

    foreach (QDateTime d, keys) {
       double time = ((double)keys.first().msecsTo(d)) / 1000;
       axisTime.append(time);
    }

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();
    foreach (double d, historyActual.values()) {
        axisActual.append(d);
        if (d < min) min = d;
        if (d > max) max = d;
    }
    foreach (double d, historySetpoint.values()) {
        axisSetpoint.append(d);
        if (d < min) min = d;
        if (d > max) max = d;
    }
    foreach (double d, historyU.values()) {
        axisU.append(d);
        if (d < min) min = d;
        if (d > max) max = d;
    }

    plot->setAxisScale(QwtPlot::yLeft, min, max);
    plot->setAxisScale(QwtPlot::xBottom, 0, keys.first().secsTo(now));

    curveActual->setData(axisTime, axisActual);
    curveSetpoint->setData(axisTime, axisSetpoint);
    curveU->setData(axisTime, axisU);

    if (historyActual.size() >= PID_WIDGET_HISTORY) {
        historyActual.remove(historyActual.keys().first());
        historySetpoint.remove(historySetpoint.keys().first());
        historyU.remove(historyU.keys().first());
    }

    plot->replot();
}
