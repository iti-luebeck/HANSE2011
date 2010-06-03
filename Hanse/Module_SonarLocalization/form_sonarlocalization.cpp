#include "form_sonarlocalization.h"
#include "ui_form_sonarlocalization.h"

#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_symbol.h>

Form_SonarLocalization::Form_SonarLocalization(QWidget *parent, Module_SonarLocalization* m) :
    QWidget(parent),
    ui(new Ui::Form_SonarLocalization)
{
    ui->setupUi(this);

    this->m = m;
    QLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
    plot = new QwtPlot(ui->plotFrame);
    l->addWidget(plot);
    ui->plotFrame->setLayout(l);

    // add curves
    curveRaw = new QwtPlotCurve("Raw data");
    curveTH = new QwtPlotCurve();
    curveVar = new QwtPlotCurve();
    curveK = new QwtPlotMarker();

    plot->setTitle("Echo data");
    plot->setAxisTitle(0,"signal");

    on_plotSelect_valueChanged(0);
}

Form_SonarLocalization::~Form_SonarLocalization()
{
    delete ui;
}

void Form_SonarLocalization::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Form_SonarLocalization::on_plotSelect_valueChanged(int )
{
    int value = ui->plotSelect->value();

    if (value<0 || value >= m->echoHistory.size())
        return;

    QDateTime time = m->echoHistory.keys().at(value);
    ui->dateTimeEdit->setDateTime(time);

    QVector<double> xData;
    for(int i=0; i<m->N; i++)
        xData.append(i);

    // copy the data into the curves
    curveRaw->setData(xData, m->echoHistory[time]);
    curveRaw->attach(plot);

    curveTH->setData(xData, m->threshHistory[time]);
    curveTH->setPen(QPen("red"));
    curveTH->attach(plot);

    curveVar->setData(xData, m->varHistory[time]);
    curveVar->setPen(QPen("yellow"));
    curveVar->attach(plot);

    int K = m->kHistory[time];
    if (K>=0) {
        curveK->setSymbol(QwtSymbol(QwtSymbol::VLine, QBrush(), QPen("green"), QSize(1,500)));
        curveK->setXValue(K);
        curveK->setYValue(0);
        curveK->attach(plot);
    }

    // finally, refresh the plot
    plot->replot();
}
