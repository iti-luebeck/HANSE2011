#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <videoInput.h>
#include <opencv/cv.h>
#include <QTimer>

using namespace cv;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTimer timer;


protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    videoInput vi;
    Mat frame;
    int camID;
    float limit;
    void getFrame(Mat &frame);
    void runFrame();


private slots:
    void on_saveApplybutton_clicked();
    void on_stopButton_clicked();
    void on_settingsButton_clicked();
    void on_startButton_clicked();
    void timerSlot();
};

#endif // MAINWINDOW_H
