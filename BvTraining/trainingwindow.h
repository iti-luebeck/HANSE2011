#ifndef TRAININGWINDOW_H
#define TRAININGWINDOW_H

#include <QMainWindow>
#include "imageprocessor.h"
#include "SVMClassifier.h"
#include "blobtraining.h"
#include "surftraining.h"

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
    QList<int> frameList;
    BlobTraining blobTraining;
    SurfTraining surfTraining;


private slots:
    void on_saveBlobButton_clicked();
    void on_loadBlobButton_clicked();
    void on_testButton_clicked();
    void on_selectButton_clicked();
    void on_loadButton_clicked();
    void on_trainButton_clicked();
};

#endif // TRAININGWINDOW_H
