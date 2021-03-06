#ifndef SLTRAININGUI_H
#define SLTRAININGUI_H

#include <QMainWindow>
#include "sonardatasourcefile.h"
#include "a.out.h"
#include "Module_SLTraining/SVMClassifier.h"
#include "sonarechodata.h"

class SonarEchoFilter;


namespace Ui {
    class SLTrainingUI;
}

class SLTrainingUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit SLTrainingUI(QWidget *parent = 0);
    ~SLTrainingUI();

private:
    Ui::SLTrainingUI *ui;
    SVMClassifier *svm;

    //simple sonar view
    QGraphicsScene scene2;
    QQueue<QLinearGradient> dataQueue;
    int simpleViewWidth;
    int selSampleWidth;
    QList<QByteArray> rawData;

    //for classification
    QList<SonarEchoData> sam;
    QList<QByteArray> viewData;
    QList<QByteArray> samples;
    QList<int> wallCandidates;
    QList<QByteArray> pSamples;
    QList<QByteArray> nSamples;
    QList<int> pWallCand;
    QList<int> nWallCand;


    float range;
    int currSample;
    int viewSamplePointer;
    QAction *actionPos;
    QAction *actionNeg;
    QAction *actionSkip;
    QAction *actionNext;

    void askForClasses();
    SonarEchoFilter *filter;
    void clearActions();
    cv::Mat cvtList2Mat();

    QList<int> classifiedData;
    QList<int> classyViewData;

    void getClassCount(int &pos, int &neg);

    void groupWallCandidates();

public slots:
    void updateSonarView2(const QList<QByteArray> samples);
    void updateSonarView3(const QList<QByteArray> samples);


private slots:

    void on_pushButton_clicked();
    void on_trainSVM_clicked();
    void on_testSVM_clicked();
    void on_selSampleWidthSlider_sliderMoved(int position);
    void on_selectSamples_clicked();
    void on_loadSonarFile_clicked();
    void on_loadSVM_clicked();
    void on_saveSVM_clicked();
    void showClassified();
    void applyHeuristic();

    //classifier slots
    void positivSample();
    void negativSample();
    void skipSample();
    //
    void showNext();
};

#endif // SLTRAININGUI_H
