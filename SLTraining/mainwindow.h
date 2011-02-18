#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <svmclassifier.h>
#include <sonardatasourcefile.h>

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
    SVMClassifier svmClassy;

    //sonar like view
    QGraphicsScene scene;
    QQueue<QGraphicsPolygonItem*> queue;
    QGraphicsItem* scanLine;
    float oldHeading;
    int oldStepSize;

    //simple view
    QGraphicsScene scene2;
    QQueue<QLinearGradient> dataQueue;
    QQueue<QGraphicsRectItem*> ritQueue;


    void askForClasses();
    //buttons
    QPushButton *pos;
    QPushButton *neg;
    QPushButton *skip;
    QPushButton *quit;
    QMessageBox box;
    int keyPressed;
    SonarEchoFilter *filter;


public slots:
    void updateSonarView(const SonarReturnData data);
    void updateSonarView2(const SonarReturnData data);

private slots:

    void on_selectSamples_clicked();
    void on_loadSonarFile_clicked();
    void on_loadSVM_clicked();
    void on_saveSVM_clicked();
    void on_loadSamples_clicked();
};

#endif // MAINWINDOW_H
