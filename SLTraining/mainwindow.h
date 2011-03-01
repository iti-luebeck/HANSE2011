#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sonardatasourcefile.h>
#include "a.out.h"
#include "SVMClassifier.h"

class SonarEchoFilter;


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SVMClassifier *svm;

    //simple sonar view
    QGraphicsScene scene2;
    QQueue<QLinearGradient> dataQueue;
    int simpleViewWidth;
    int selSampleWidth;

    //for classification
    QList<QByteArray> viewData;
    QList<QByteArray> samples;
    QList<int> wallCandidates;
    QList<QByteArray> pSamples;
    QList<QByteArray> nSamples;
    float range;
    int currSample;
    QAction *actionPos;
    QAction *actionNeg;
    QAction *actionSkip;

    void askForClasses();
    SonarEchoFilter *filter;
    void clearActions();
    cv::Mat cvtList2Mat();

public slots:
    void updateSonarView2(const QList<QByteArray> samples);


private slots:

    void on_pushButton_clicked();
    void on_trainSVM_clicked();
    void on_testSVM_clicked();
    void on_selSampleWidthSlider_sliderMoved(int position);
    void on_selectSamples_clicked();
    void on_loadSonarFile_clicked();
    void on_loadSVM_clicked();
    void on_saveSVM_clicked();

    //classifier slots
    void positivSample();
    void negativSample();
    void skipSample();
};

#endif // MAINWINDOW_H
