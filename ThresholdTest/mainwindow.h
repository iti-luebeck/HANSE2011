#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    void loadImage(QString fileName);
    void refreshThreshold();
    void moments( IplImage *image, double &m10, double &m01, double &mu11, double &mu02, double &mu20 );

private:
    Ui::MainWindow *ui;

    int colorspace;
    int channel;
    int method;
    bool inverted;

    int lowThreshold;
    int highThreshold;

    IplImage *rgbImage;
    IplImage *hsvImage;
    IplImage *grayscaleImage;
    IplImage *R;
    IplImage *G;
    IplImage *B;
    IplImage *H;
    IplImage *S;
    IplImage *V;
    IplImage *YB;
    IplImage *binary;

    bool playing;
    int count;
    QStringList files;

private slots:
    void on_bluyelRadiButton_toggled(bool checked);
    void on_kmeansRadioButton_toggled(bool checked);
    void on_otsuRadioButton_toggled(bool checked);
    void on_normalRadioButton_toggled(bool checked);
    void on_bluyelRadiButton_clicked();
    void on_playButton_clicked();
    void on_invertCheckBox_toggled(bool checked);
    void on_loadButton_clicked();
    void on_hsvRadioButton_toggled(bool checked);
    void on_rgbRadioButton_toggled(bool checked);
    void on_channel3RadioButton_toggled(bool checked);
    void on_channel2RadioButton_toggled(bool checked);
    void on_channel1RadioButton_toggled(bool checked);
    void on_grayRadioButton_toggled(bool checked);
    void on_horizontalSlider_2_sliderMoved(int position);
    void on_horizontalSlider_sliderMoved(int position);

    void nextImage();
};

#endif // MAINWINDOW_H
