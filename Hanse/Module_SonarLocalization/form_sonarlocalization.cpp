#include "form_sonarlocalization.h"
#include "ui_form_sonarlocalization.h"
#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_symbol.h>
#include <qwt-qt4/qwt_legend.h>
#include <qfiledialog.h>

#include "sonarechofilter.h"
#include "sonarparticlefilter.h"
#include "Module_SLTraining/sltrainingui.h"

Form_SonarLocalization::Form_SonarLocalization(QWidget *parent, Module_SonarLocalization* m) :
    QWidget(parent),
    ui(new Ui::Form_SonarLocalization)
{
    ui->setupUi(this);

    this->m = m;

    currentPos = NULL;

    createPlot();

    scene = new QGraphicsScene(parent);
//    createMap();

    setFields();

    on_plotSelect_valueChanged(0);

    sonarEchoDataList.clear();

    this->ui->graphicsView->setScene(&scene2);
    scene2.clear();
    scene2.setBackgroundBrush(QBrush(Qt::white));

    this->ui->unfilteredView->setScene(&sceneUnfiltered);
    sceneUnfiltered.clear();
    sceneUnfiltered.setBackgroundBrush(QBrush(Qt::white));

    sceneRaw.clear();
    sceneRaw.setBackgroundBrush(QBrush(Qt::white));
    this->ui->rawView->setScene(&sceneRaw);

    connect(m,SIGNAL(newSonarPlotData(QList<SonarEchoData>)),this,SLOT(updateSonarViewList(QList<SonarEchoData>)));
//    connect(m->pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEstimate(QVector3D)));
//    connect(m->pf, SIGNAL(working(bool)), this, SLOT(particleFilterStatus(bool)));
}

void Form_SonarLocalization::createPlot()
{
    QLayout* l = new QBoxLayout(QBoxLayout::LeftToRight);
    plot = new QwtPlot(ui->plotFrame);
    l->addWidget(plot);
    ui->plotFrame->setLayout(l);

    // add curves
    curveRaw = new QwtPlotCurve("Raw data");
    curveFiltered = new QwtPlotCurve("Filtered data");
    curveTH = new QwtPlotCurve("threshold");
    curveVar = new QwtPlotCurve("stdDev");
    curveMean = new QwtPlotCurve("prev mean");
    curveK = new QwtPlotMarker();
    curveVarTH = new QwtPlotMarker();

    plot->setTitle("Echo data");
    plot->setAxisTitle(0,"signal");
    plot->setAxisScale(QwtPlot::yLeft,0,1);

    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box|| QFrame::Sunken);
    plot->insertLegend(legend,QwtPlot::BottomLegend);

}

void Form_SonarLocalization::createMap()
{
    ui->mapView->setScene(scene);
    ui->mapView->scale(5,5);
    ui->PFactive->setVisible(false);

    QImage satImg(m->getSettingsValue("satImgFile").toString());
    QGraphicsPixmapItem *result = scene->addPixmap(QPixmap::fromImage(satImg));
    result->setPos(0,0);
    result->setZValue(-1);
    //0.2
    result->setScale(m->getSettingsValue("scaleMap").toReal());

    QVector<SonarParticle> particles = m->pf.getParticles();
    foreach (SonarParticle p, particles) {
        particleItems.append(scene->addEllipse(p.getX(), p.getY(), 1, 1, QPen(QColor("green"))));
    }

    // draw map
    foreach (QVector2D p, m->pf.getMapPoints()) {
        scene->addEllipse(p.x(), p.y(), 1 ,1, QPen(QColor("yellow")));
    }
}

void Form_SonarLocalization::setFields()
{
    ui->config_mapFile->setText(m->getSettingsValue("mapFile").toString());
    ui->config_satImage->setText(m->getSettingsValue("satImgFile").toString());
    ui->scaleMap->setText(m->getSettingsValue("scaleMap", 0.2).toString());

    ui->debug->setChecked((m->getSettingsValue("debug", false).toBool()));

    ui->darknessCnt->setText((m->getSettingsValue("darknessCnt", 20).toString()));
    ui->singlePointCB->setChecked(m->getSettingsValue("singlePoint",true).toBool());
    ui->deltaKHCB->setChecked(m->getSettingsValue("deltaKH",true).toBool());

    ui->imgMinPixels->setText((m->getSettingsValue("imgMinPixels", 10).toString()));
    ui->controlVariance->setText((m->getSettingsValue("controlVariance", "2;2;1").toString()));
    ui->initVariance->setText((m->getSettingsValue("initVariance", "5;5;2").toString()));
    ui->distanceCutoff->setText((m->getSettingsValue("distanceCutoff", 10000).toString()));
    ui->particleCount->setText((m->getSettingsValue("particleCount", 1000).toString()));
    ui->boltzmann->setText((m->getSettingsValue("observationVariance", 10).toString()));

    ui->groupingDarknessCnt->setText(m->getSettingsValue("groupingDarkness", 10).toString());
    ui->groupingMaxArea->setText(m->getSettingsValue("groupingMaxArea", 360).toString());

    ui->gradMaxVal->setText(m->getSettingsValue("gradientMaxVal", 0.1).toString());
    ui->gradMaxIdx->setText(m->getSettingsValue("gradientMaxIdx", 5).toString());
    ui->histMaxVal->setText(m->getSettingsValue("histMaxVal", 1).toString());

    ui->xsensBox->setChecked(m->getSettingsValue("use xsens", false).toBool());
    ui->obsDistEdit->setText(m->getSettingsValue("min obs dist", 2).toString());
    ui->filterLengthBox->setText(m->getSettingsValue("filter lengths", "2;4;8").toString());
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

    if (value<0 || value >= m->filter.rawHistory.size())
        return;

    QDateTime time = m->filter.rawHistory.keys().at(value);
    ui->dateTimeEdit->setDateTime(time);

    QVector<double> xData;
    for (int i = 0; i < 250; i++)
        xData.append(i/5.0);

    // copy the data into the curves
    curveRaw->setData(xData, m->filter.rawHistory[time]);
    curveRaw->attach(plot);

    curveFiltered->setData(xData, m->filter.filteredHistory[time]);
    curveFiltered->setPen(QPen("green"));
//    curveFiltered->attach(plot);

    curveTH->setData(xData, m->filter.threshHistory[time]);
    curveTH->setPen(QPen("red"));
    curveTH->attach(plot);

    curveVar->setData(xData, m->filter.varHistory[time]);
    curveVar->setPen(QPen("brown"));
    curveVar->attach(plot);

    curveMean->setData(xData, m->filter.meanHistory[time]);
    curveMean->setPen(QPen("blue"));
    curveMean->attach(plot);

    int K = m->filter.kHistory[time];
    if (K>=0) {
        curveK->setSymbol(QwtSymbol(QwtSymbol::VLine, QBrush(), QPen("green"), QSize(1,500)));
//        curveK->setXValue(15.0);
        curveK->setXValue(K/5.0);
        curveK->setYValue(0);
        curveK->attach(plot);
    }

    curveVarTH->setSymbol(QwtSymbol(QwtSymbol::HLine, QBrush(), QPen("green"), QSize(2000,1)));
    curveVarTH->setXValue(0);
    curveVarTH->setYValue(m->getSettingsValue("varTH").toFloat());
    curveVarTH->attach(plot);

    // finally, refresh the plot
    plot->replot();
}

void Form_SonarLocalization::on_pushButton_clicked()
{
    m->setSettingsValue("mapFile", ui->config_mapFile->text());
    m->setSettingsValue("satImgFile", ui->config_satImage->text());

    m->setSettingsValue("scaleMap",ui->scaleMap->text());

    m->setSettingsValue("debug", ui->debug->isChecked());

    m->setSettingsValue("darknessCnt", ui->darknessCnt->text());
    m->setSettingsValue("deltaKH",ui->deltaKHCB->isChecked());
    m->setSettingsValue("singlePoint",ui->singlePointCB->isChecked());

    m->setSettingsValue("imgMinPixels", ui->imgMinPixels->text());
    m->setSettingsValue("controlVariance", ui->controlVariance->text());
    m->setSettingsValue("initVariance", ui->initVariance->text());
    m->setSettingsValue("distanceCutoff", ui->distanceCutoff->text());
    m->setSettingsValue("particleCount", ui->particleCount->text());
    m->setSettingsValue("observationVariance", ui->boltzmann->text());

    m->setSettingsValue("groupingDarkness",ui->groupingDarknessCnt->text());
    m->setSettingsValue("groupingMaxArea",ui->groupingMaxArea->text());

    m->setSettingsValue("gradientMaxVal",ui->gradMaxVal->text());
    m->setSettingsValue("gradientMaxIdx",ui->gradMaxIdx->text());
    m->setSettingsValue("histMaxVal",ui->histMaxVal->text());

    m->setSettingsValue("use xsens",ui->xsensBox->isChecked());
    m->setSettingsValue("min obs dist",ui->obsDistEdit->text().toFloat());

    m->setSettingsValue("filter lengths", ui->filterLengthBox->text());

    m->reset();
}

void Form_SonarLocalization::newPositionEstimate(QVector3D __attribute__ ((unused)) e)
{
    if (!ui->update->isChecked())
        return;

    foreach (QGraphicsEllipseItem* it, particleItems) {
        delete it;
    }
    particleItems.clear();
    QVector<SonarParticle> particles = m->pf.getParticles();
    foreach (SonarParticle p, particles) {
        particleItems.append(scene->addEllipse(p.getX(), p.getY(), 1,1,QPen(QColor("green"))));
    }

    on_spinBox_valueChanged(ui->spinBox->value());

}

void Form_SonarLocalization::on_spinBox_valueChanged(int p)
{
    if (!ui->update->isChecked())
        return;

    QList<QVector2D> z = m->pf.getLatestObservation();
    if (z.size()==0 || m->pf.getParticleCount() < p)
        return;

    m->logger->debug("");
    //ui->time->setTime( QTime::currentTime() );
    ui->no_points->setText(QString::number(z.size()));

    foreach (QGraphicsEllipseItem* it, volatileItems) {
        delete it;
    }
    volatileItems.clear();

    SonarParticle particle = m->pf.getParticles()[p];
    m->logger->debug("particle: X="+QString::number(particle.getX())+",Y="+QString::number(particle.getY())+",Z="
                     +QString::number(particle.getTheta())+",W="+QString::number(particle.getWeight()));

    for (int i=0; i<z.size(); i++) {
        QVector2D o = z[i];

        QTransform rotM = QTransform().rotate(particle.getTheta()/M_PI*180) * QTransform().translate(particle.getX(), particle.getY());
        QPointF q = rotM.map(o.toPointF());

        volatileItems.append(scene->addEllipse(q.x(), q.y(), 1,1,QPen(QColor("blue"))));
    }

    if (currentPos != NULL)
        delete currentPos;
    currentPos = scene->addEllipse(m->pf.getParticles()[p].getX(), m->pf.getParticles()[p].getY(), 1,1,QPen(QColor("red")));
    currentPos->setZValue(10);
}

void Form_SonarLocalization::particleFilterStatus(bool status) {
    ui->PFactive->setVisible(status);
}

void Form_SonarLocalization::on_selMap_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Map file", ui->config_mapFile->text(), "Selfmade map file (*.png *.jpg *.bmp)");

    if (fileName.length()>0)
        ui->config_mapFile->setText(fileName);
}

void Form_SonarLocalization::on_selSat_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open satellite image", ui->config_satImage->text(), "satellite image file (*.png *.jpg *.bmp)");

    if (fileName.length()>0)
        ui->config_satImage->setText(fileName);
}

void Form_SonarLocalization::updateSonarViewList(QList<SonarEchoData> list)
{
    sonarEchoDataList = list;
//    sonarEchoDataList.clear();
//    while (!list.empty()) {
//        SonarEchoData d = list.takeFirst();
//        sonarEchoDataList.append(d);
//    }

    if(this->ui->enableUnfilteredOutput->isChecked())
        this->updateSonarViewUnfiltered();
    if(this->ui->enableFilteredView->isChecked())
        this->updateSonarView();
    if(this->ui->enableRawView->isChecked())
        this->updateSonarViewRaw();
}

void Form_SonarLocalization::updateSonarView()
{
    scene2.clear();
    scene2.setBackgroundBrush(QBrush(Qt::white));

    float width = ui->graphicsView->width() - 5;
    float height = ui->graphicsView->height() - 5;
    float stepWidth = width / sonarEchoDataList.length();

    for (int j = 0; j < sonarEchoDataList.length(); j++) {
        QList<float> data = sonarEchoDataList[j].getGradient();
        QLinearGradient gi(0, 0, 0, data.length());
        for (int i = 0; i < data.length(); i++) {
            int skalarM = 1;
            int wc = sonarEchoDataList[j].getWallCandidate();

            if ((wc > 0)  && (i > wc - skalarM ) && (i < wc + skalarM)) {
                gi.setColorAt(1.0 * i / data.length(), QColor(0,0,0));
            } else {
                gi.setColorAt(1.0 * i / data.length(), QColor(255,255,255));
            }

        }

        scene2.addRect(j*stepWidth, 1, stepWidth, height, Qt::NoPen, QBrush(gi));
    }
}

void Form_SonarLocalization::updateSonarViewUnfiltered()
{
    sceneUnfiltered.clear();
    sceneUnfiltered.setBackgroundBrush(QBrush(Qt::white));

    float width = ui->graphicsView->width() - 5;
    float height = ui->graphicsView->height() - 5;
    float stepWidth = width / sonarEchoDataList.length();

    // Find maximum value for scaling.
    float maxVal = 0.00001;
    for (int j = 0; j < sonarEchoDataList.length(); j++) {
        QList<float> data = sonarEchoDataList[j].getGradient();
        for (int i = 0; i < data.size(); i++) {
            if (data[i] > maxVal) {
                maxVal = data[i];
            }

        }
    }

    for (int j = 0; j < sonarEchoDataList.length(); j++) {
        QList<float> data = sonarEchoDataList[j].getGradient();
        QLinearGradient gi(0, 0, 0, data.length());
        for (int i = 0; i < data.length(); i++) {
            int col = 255 - 255 * data[i] / maxVal;
            if (col < 0) col = 0;
            if (col > 255) col = 255;
            gi.setColorAt(1.0 * i / data.length(), QColor(col,col,col));
        }

        sceneUnfiltered.addRect(j*stepWidth, 1, stepWidth, height, Qt::NoPen, QBrush(gi));
    }
}

void Form_SonarLocalization::updateSonarViewRaw()
{
    sceneRaw.clear();
    sceneRaw.setBackgroundBrush(QBrush(Qt::white));

    float width = ui->rawView->width() - 5;
    float height = ui->rawView->height() - 5;
    float stepWidth = width / sonarEchoDataList.length();

    for (int j = 0; j < sonarEchoDataList.length(); j++) {
        QByteArray data = sonarEchoDataList[j].getFiltered();
        QLinearGradient gi(0, 0, 0, data.length());
        for (int i = 0; i < data.length(); i++) {
            int col = 255 - 2 * (int)data[i];
            if (col < 0) col = 0;
            if (col > 255) col = 255;
            gi.setColorAt(1.0 * i / data.length(), QColor(col, col, col));
        }
        sceneRaw.addRect(j*stepWidth, 1, stepWidth, height, Qt::NoPen, QBrush(gi));
    }
}

void Form_SonarLocalization::on_selSVM_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open SVM Classifier", ui->configure_SVM->text());

    if (fileName.length()>0)
    {
        ui->configure_SVM->setText(fileName);
        this->m->setSettingsValue("Path2SVM",fileName);
    }

}

void Form_SonarLocalization::on_sltraining_clicked()
{
    SLTrainingUI slt(&m->sonarEchoFilter());
    QTimer::singleShot(0,&slt,SLOT(show()));
}
