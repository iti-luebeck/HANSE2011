#include "tcl_form.h"
#include "ui_tcl_form.h"

#include <qwt-qt4/qwt_legend.h>

TCL_Form::TCL_Form(Module_ThrusterControlLoop *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TCL_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->p_up->setText(module->getSettings().value(      "p_up").toString());
    ui->p_down->setText(module->getSettings().value(    "p_down").toString());
    ui->maxSpU->setText(module->getSettings().value(    "maxSpU").toString());
    ui->maxSpD->setText(module->getSettings().value(    "maxSpD").toString());
    ui->neutrSpD->setText(module->getSettings().value(  "neutrSpD").toString());
    ui->maxDepthError->setText(module->getSettings().value(  "maxDepthError").toString());
    ui->forceUnpauseError->setText(module->getSettings().value("forceUnpauseError").toString());

    ui->horizSpM_exp->setChecked( module->getSettings().value("horizSpM_exp").toBool() );

    // add curves
    QLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
    plot = new QwtPlot(ui->frame);
    l->addWidget(plot);
    ui->frame->setLayout(l);

    plot->setAxisScale(QwtPlot::yLeft,-1,2);
    plot->setAxisScale(QwtPlot::xBottom,0,300);

    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box|| QFrame::Sunken);
    plot->insertLegend(legend,QwtPlot::BottomLegend);

    curveIst = new QwtPlotCurve("ist");
    curveSoll = new QwtPlotCurve("soll");
    curveThruster = new QwtPlotCurve("thruster");

    curveIst->attach(plot);
    curveIst->setPen(QPen("red"));
    curveSoll->attach(plot);
    curveSoll->setPen(QPen("black"));
    curveThruster->attach(plot);
    curveThruster->setPen(QPen("blue"));

    plot->setTitle("depth control");
    plot->setAxisTitle(QwtPlot::xBottom,"time (s)");
    plot->setAxisTitle(QwtPlot::yLeft,"depth (m)");

    connect(module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));

}

TCL_Form::~TCL_Form()
{
    delete ui;
}

void TCL_Form::changeEvent(QEvent *e)
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

void TCL_Form::on_save_clicked()
{
    module->getSettings().setValue("p_up",      ui->p_up->text().toFloat());
    module->getSettings().setValue("p_down",    ui->p_down->text().toFloat());
    module->getSettings().setValue("maxSpU",    ui->maxSpU->text().toFloat());
    module->getSettings().setValue("maxSpD",    ui->maxSpD->text().toFloat());
    module->getSettings().setValue("neutrSpD",  ui->neutrSpD->text().toFloat());
    module->getSettings().setValue("maxDepthError",  ui->maxDepthError->text().toFloat());
    module->getSettings().setValue("forceUnpauseError", ui->forceUnpauseError->text());

    module->getSettings().setValue("horizSpM_exp", ui->horizSpM_exp->isChecked() );
    module->getSettings().setValue("ignoreHealth", ui->ignoreHealth->isChecked() );

    module->updateConstantsFromInitNow();
}

void TCL_Form::dataChanged(RobotModule *mod)
{
    if (!ui->updatePlot->isChecked()) {
        return;
    }

    QList<QDateTime> keys = module->historyIst.keys();

    QVector<double> axisTime;
    QVector<double> axisIst;
    QVector<double> axisSoll;
    QVector<double> axisThruster;


    foreach (QDateTime d, keys) {
       int time = keys.first().secsTo(d);
       axisTime.append(time);
    }

    foreach (float d, module->historyIst.values()) {
        axisIst.append(d);
    }
    foreach (float d, module->historySoll.values()) {
        axisSoll.append(d);
    }
    foreach (float d, module->historyThrustCmd.values()) {
        axisThruster.append(d);
    }

    curveIst->setData(axisTime, axisIst);
    curveSoll->setData(axisTime, axisSoll);
    curveThruster->setData(axisTime, axisThruster);

    plot->replot();

    ui->elCnt->setText(QString::number(axisTime.size()));

}
