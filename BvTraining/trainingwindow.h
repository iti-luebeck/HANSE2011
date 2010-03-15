#ifndef TRAININGWINDOW_H
#define TRAININGWINDOW_H

#include <QMainWindow>
#include "imageprocessor.h"
#include "SVMClassifier.h"

namespace Ui {
    class TrainingWindow;
}

class TrainingWindow : public QMainWindow {
    Q_OBJECT
public:
    TrainingWindow(QWidget *parent = 0);
    ~TrainingWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrainingWindow *ui;

    QString videoFile;
    ImageProcessor ip;
    QList<int> frameList;
    SVMClassifier *svm;

private slots:
    void on_testButton_clicked();
    void on_selectButton_clicked();
    void on_loadButton_clicked();
    void on_trainButton_clicked();
};

#endif // TRAININGWINDOW_H
