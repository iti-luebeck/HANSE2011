#include "form_sonarlocalization.h"
#include "ui_form_sonarlocalization.h"

#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_symbol.h>
#include <qwt-qt4/qwt_legend.h>

#include "sonarechofilter.h"
#include "sonarparticlefilter.h"

Form_SonarLocalization::Form_SonarLocalization(QWidget *parent, Module_SonarLocalization* m) :
    QWidget(parent),
    ui(new Ui::Form_SonarLocalization)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(parent);
    ui->mapView->setScene(scene);
    ui->mapView->scale(5,5);

    this->m = m;
    QLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
    plot = new QwtPlot(ui->plotFrame);
    l->addWidget(plot);
    ui->plotFrame->setLayout(l);

    currentPos = NULL;

    // add curves
    curveRaw = new QwtPlotCurve("Raw data");
    curveFiltered = new QwtPlotCurve("Filtered data");
    curveTH = new QwtPlotCurve("threshold");
    curveVar = new QwtPlotCurve("stdDev");
    curveMean = new QwtPlotCurve("prev mean");
    curveK = new QwtPlotMarker();

    plot->setTitle("Echo data");
    plot->setAxisTitle(0,"signal");

    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box|| QFrame::Sunken);
    plot->insertLegend(legend,QwtPlot::BottomLegend);

    on_plotSelect_valueChanged(0);

    ui->config_mapFile->setText(m->getSettings().value("mapFile").toString());
    ui->config_satImage->setText(m->getSettings().value("satImgFile").toString());


    QImage satImg(m->getSettings().value("satImgFile").toString());
    QGraphicsPixmapItem *result = scene->addPixmap(QPixmap::fromImage(satImg));
    result->setPos(0,0);
    result->setZValue(-1);
    result->setScale(0.2);

    QVector<QVector4D> particles = m->pf->getParticles();
    foreach (QVector4D p, particles) {
        particleItems.append(scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("green"))));
    }

    // draw map
    foreach (QVector2D p, m->pf->mapPoints) {
        scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("yellow")));
    }

    connect(m->filter, SIGNAL(newImage(QVector<QVector2D>)), this, SLOT(newImage(QVector<QVector2D>)));
    connect(m->pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEstimate(QVector3D)));
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

    if (value<0 || value >= m->filter->rawHistory.size())
        return;

    QDateTime time = m->filter->rawHistory.keys().at(value);
    ui->dateTimeEdit->setDateTime(time);

    QVector<double> xData;
    for(int i=0; i<m->filter->N; i++)
        xData.append(i);

    // copy the data into the curves
    curveRaw->setData(xData, m->filter->rawHistory[time]);
    curveRaw->attach(plot);

    curveFiltered->setData(xData, m->filter->filteredHistory[time]);
    curveFiltered->setPen(QPen("brown"));
    curveFiltered->attach(plot);

    curveTH->setData(xData, m->filter->threshHistory[time]);
    curveTH->setPen(QPen("red"));
    curveTH->attach(plot);

    curveVar->setData(xData, m->filter->varHistory[time]);
    curveVar->setPen(QPen("yellow"));
    curveVar->attach(plot);

    curveMean->setData(xData, m->filter->meanHistory[time]);
    curveMean->setPen(QPen("blue"));
    curveMean->attach(plot);

    int K = m->filter->kHistory[time];
    if (K>=0) {
        curveK->setSymbol(QwtSymbol(QwtSymbol::VLine, QBrush(), QPen("green"), QSize(1,500)));
        curveK->setXValue(K);
        curveK->setYValue(0);
        curveK->attach(plot);
    }

    // finally, refresh the plot
    plot->replot();
}

void Form_SonarLocalization::newImage(QVector<QVector2D> observations)
{
    if (observations.size()==0)
        return;

    ui->time->setTime( QTime::currentTime() );
    ui->no_points->setText(QString::number(observations.size()));

    foreach (QGraphicsEllipseItem* it, volatileItems) {
        delete it;
    }
    volatileItems.clear();

    for (int i=0; i<observations.size(); i++) {
        QVector2D o = observations[i];
        volatileItems.append(scene->addEllipse(o.x(), o.y(), 1,1,QPen(QColor("blue"))));
    }
}

void Form_SonarLocalization::on_pushButton_clicked()
{
    m->getSettings().setValue("mapFile", ui->config_mapFile->text());
    m->getSettings().setValue("satImgFile", ui->config_satImage->text());
}

void Form_SonarLocalization::newPositionEstimate(QVector3D e)
{
    m->logger->debug("Got a new position estimate.");
    if (currentPos != NULL)
        delete currentPos;
    currentPos = scene->addEllipse(e.x(), e.y(), 1,1,QPen(QColor("red")));

    foreach (QGraphicsEllipseItem* it, volatileItems) {
        it->translate(e.x(), e.y());
    }

    foreach (QGraphicsEllipseItem* it, particleItems) {
        delete it;
    }
    particleItems.clear();
    QVector<QVector4D> particles = m->pf->getParticles();
    foreach (QVector4D p, particles) {
        particleItems.append(scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("green"))));
    }
}
