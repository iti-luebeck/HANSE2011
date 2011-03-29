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

    this->ui->graphicsView->setScene(&scene2);
    QLinearGradient gi(0,0,0,279);
    gi.setColorAt(0,QColor("black"));
    gi.setColorAt(1,QColor("black"));

    scene2.clear();
    dataQueue.clear();
    sonarEchoDataList.clear();
    for(int i =0; i<100; i++)
    {
        dataQueue.append(gi);

    }

    //unfiltered View
    this->ui->unfilteredView->setScene(&sceneUnfiltered);
    sceneUnfiltered.clear();
    dataQueueUnfiltered.clear();
    sonarEchoDataUnfilteredList.clear();
    for(int i =0; i<100; i++)
        dataQueueUnfiltered.append(gi);


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
    result->setScale(0.2);

    QVector<QVector4D> particles = m->pf.getParticles();
    foreach (QVector4D p, particles) {
        particleItems.append(scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("green"))));
    }

    // draw map
    foreach (QVector2D p, m->pf.getMapPoints()) {
        scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("yellow")));
    }
}

void Form_SonarLocalization::setFields()
{

    ui->config_mapFile->setText(m->getSettingsValue("mapFile").toString());
    ui->config_satImage->setText(m->getSettingsValue("satImgFile").toString());

    ui->debug->setChecked((m->getSettingsValue("debug").toBool()));

    ui->gaussFactor->setText((m->getSettingsValue("gaussFactor").toString()));

    ui->darknessCnt->setText((m->getSettingsValue("darknessCnt").toString()));
    ui->swipedArea->setText((m->getSettingsValue("swipedArea").toString()));

    ui->wallWindowSize->setText((m->getSettingsValue("wallWindowSize").toString()));
    ui->varTH->setText((m->getSettingsValue("varTH").toString()));
    ui->largePeakTH->setText((m->getSettingsValue("largePeakTH").toString()));
    ui->meanBehindTH->setText((m->getSettingsValue("meanBehindTH").toString()));

    ui->imgMinPixels->setText((m->getSettingsValue("imgMinPixels").toString()));
    ui->controlVariance->setText((m->getSettingsValue("controlVariance").toString()));
    ui->initVariance->setText((m->getSettingsValue("initVariance").toString()));
    ui->distanceCutoff->setText((m->getSettingsValue("distanceCutoff").toString()));
    ui->particleCount->setText((m->getSettingsValue("particleCount").toString()));
    ui->boltzmann->setText((m->getSettingsValue("boltzmann").toString()));

    ui->a1->setText(m->getSettingsValue("a1").toString());
    ui->a2->setText(m->getSettingsValue("a2").toString());

    ui->configure_SVM->setText(m->getSettingsValue("Path2SVM").toString());
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
    for(int i=0; i<m->filter.N; i++)
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

    m->setSettingsValue("debug", ui->debug->isChecked());

    m->setSettingsValue("gaussFactor", ui->gaussFactor->text());

    m->setSettingsValue("darknessCnt", ui->darknessCnt->text());
    m->setSettingsValue("swipedArea", ui->swipedArea->text());

    m->setSettingsValue("wallWindowSize", ui->wallWindowSize->text());
    m->setSettingsValue("varTH", ui->varTH->text());
    m->setSettingsValue("largePeakTH", ui->largePeakTH->text());
    m->setSettingsValue("meanBehindTH", ui->meanBehindTH->text());

    m->setSettingsValue("imgMinPixels", ui->imgMinPixels->text());
    m->setSettingsValue("controlVariance", ui->controlVariance->text());
    m->setSettingsValue("initVariance", ui->initVariance->text());
    m->setSettingsValue("distanceCutoff", ui->distanceCutoff->text());
    m->setSettingsValue("particleCount", ui->particleCount->text());
    m->setSettingsValue("boltzmann", ui->boltzmann->text());

    m->setSettingsValue("a1",  ui->a1->text());
    m->setSettingsValue("a2",  ui->a2->text());

    m->reset();
}

void Form_SonarLocalization::newPositionEstimate(QVector3D e)
{
    if (!ui->update->isChecked())
        return;

    foreach (QGraphicsEllipseItem* it, particleItems) {
        delete it;
    }
    particleItems.clear();
    QVector<QVector4D> particles = m->pf.getParticles();
    foreach (QVector4D p, particles) {
        particleItems.append(scene->addEllipse(p.x(), p.y(), 1,1,QPen(QColor("green"))));
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

    QVector4D particle = m->pf.getParticles()[p];
    m->logger->debug("particle: X="+QString::number(particle.x())+",Y="+QString::number(particle.y())+",Z="
                     +QString::number(particle.z())+",W="+QString::number(particle.w()));

    for (int i=0; i<z.size(); i++) {
        QVector2D o = z[i];

        QTransform rotM = QTransform().rotate(particle.z()/M_PI*180) * QTransform().translate(particle.x(), particle.y());
        QPointF q = rotM.map(o.toPointF());

        volatileItems.append(scene->addEllipse(q.x(), q.y(), 1,1,QPen(QColor("blue"))));
    }

    if (currentPos != NULL)
        delete currentPos;
    currentPos = scene->addEllipse(m->pf.getParticles()[p].x(), m->pf.getParticles()[p].y(), 1,1,QPen(QColor("red")));
    currentPos->setZValue(10);
}

void Form_SonarLocalization::particleFilterStatus(bool status) {
    ui->PFactive->setVisible(status);
}

void Form_SonarLocalization::on_selMap_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open Map file", ui->config_mapFile->text(), "Selfmade map file (*.png *.jpg)");

    if (fileName.length()>0)
        ui->config_mapFile->setText(fileName);
}

void Form_SonarLocalization::on_selSat_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open satellite image", ui->config_satImage->text(), "satellite image file (*.png *.jpg)");

    if (fileName.length()>0)
        ui->config_satImage->setText(fileName);
}

void Form_SonarLocalization::updateSonarView(SonarEchoData data)
{
//    qDebug() << "new candidate " << WallCandidate;
    int viewWidth = 100;
    sonarEchoDataList.append(data);
    while(sonarEchoDataList.size() < viewWidth)
        sonarEchoDataList.append(data);

    if(sonarEchoDataList.size() > viewWidth)
        sonarEchoDataList.removeFirst();

    float range = sonarEchoDataList.first().getRange();

    float n = 250;
    scene2.clear();
    int height = ui->graphicsView->height()-1;
    float faktor = (height/range);
//    if(range > 20.0)
//        faktor = faktor * 10;
    if(true)
    {
        for(int i = 1; i<range+1; i++)
            scene2.addLine(0,(i*faktor),viewWidth,(i*faktor),QPen(QColor(200,83,83,255)))->setZValue(10);

        for(int j=0; j<viewWidth; j++)
        {
            QLinearGradient gi(0,0,0,279);
            for (int i = 0; i < n; i++) {
                int skalarM = 1;
                int cl = sonarEchoDataList[j].getClassLabel();
                int wc = sonarEchoDataList[j].getWallCandidate();
//                qDebug() << "current wc " << wc;

                //mark groups
//                if(i<=2)
//                {
//                if(echoList.last().getGroupID()%2 == 0)
//                    gi.setColorAt(1.0*i/n,QColor(255,0,0));
//                else
//                    gi.setColorAt(1.0*i/n,QColor(0,0,255));
//                }

                if(((cl != 0) && (wc != -1)  && (i > wc- skalarM ) && (i < wc + skalarM)))
                    gi.setColorAt(1.0*i/n,QColor(0,255,0));
                else
                    gi.setColorAt(1.0*i/n,QColor(0,0,0));

            }
            dataQueue.append(gi);
            dataQueue.pop_front();

        }

        for(int i =0; i<viewWidth; i++)
            scene2.addRect(i,1,1,274,(Qt::NoPen),QBrush(dataQueue[i]));
    }
}

void Form_SonarLocalization::updateSonarViewUnfiltered(SonarEchoData unfiltered)
{
    int viewWidth = 100;
    sonarEchoDataUnfilteredList.append(unfiltered);
    while(sonarEchoDataUnfilteredList.size() < viewWidth)
        sonarEchoDataUnfilteredList.append(unfiltered);

    if(sonarEchoDataUnfilteredList.size() > viewWidth)
        sonarEchoDataUnfilteredList.removeFirst();

    float range = sonarEchoDataUnfilteredList.first().getRange();
    float n = 250;

    sceneUnfiltered.clear();
    int height = ui->unfilteredView->height()-1;
    float faktor = (height/range);
//    if(range > 20.0)
//        faktor = faktor * 10;
    if(true)
    {

        for(int i = 1; i<range+1; i++)
            sceneUnfiltered.addLine(0,(i*faktor),viewWidth,(i*faktor),QPen(QColor(200,83,83,255)))->setZValue(10);

        for(int j=0; j<viewWidth; j++)
        {
            QLinearGradient gi(0,0,0,279);
            for (int i = 0; i < n; i++) {
//                QByteArray data = curDataSet.at(j);
                QByteArray data = sonarEchoDataUnfilteredList[j].getFiltered();
//                QByteArray data = sam[viewSamplePointer+j].getRawData();
                char b = data[i];
//                if(j == (curDataSet.length()/2))

                int wws = 2;
                int skalarM = 25;

//                if((j == (viewWidth/2)))
//                {
//                    int wc = sonarEchoDataUnfilteredList[j].getWallCandidate();
////                    int wc = wallCandidates.first();
//                     if((i >= (wc - wws)) && (i <= (wc + wws)))
//                        gi.setColorAt(1.0*i/n,QColor(skalarM*b,0,0));
//                     else
//                         gi.setColorAt(1.0*i/n,QColor(0,0,skalarM*b));
//                }
//                else
                    gi.setColorAt(1.0*i/n,QColor(0,skalarM*b,0));
            }
            dataQueueUnfiltered.append(gi);
            dataQueueUnfiltered.pop_front();

        }

        for(int i =0; i<viewWidth; i++)
            sceneUnfiltered.addRect(i,1,1,274,(Qt::NoPen),QBrush(dataQueueUnfiltered[i]));

    }
}


void Form_SonarLocalization::on_enableUnfilteredOutput_clicked(bool checked)
{
    disconnect(m,SIGNAL(newSonarEchoData(SonarEchoData)),this,SLOT(updateSonarViewUnfiltered(SonarEchoData)));
    if(checked)
        connect(m,SIGNAL(newSonarEchoData(SonarEchoData)),this,SLOT(updateSonarViewUnfiltered(SonarEchoData)));
}

void Form_SonarLocalization::on_enableFilteredView_clicked(bool checked)
{
     disconnect(m,SIGNAL(newSonarEchoData(SonarEchoData)),this,SLOT(updateSonarView(SonarEchoData)));
     if(checked)
         connect(m,SIGNAL(newSonarEchoData(SonarEchoData)),this,SLOT(updateSonarView(SonarEchoData)));

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
