#ifndef SVMCLASSIFIER_H
#define SVMCLASSIFIER_H

#include <opencv/cv.h>
#include <opencv/ml.h>
#include <QString>
#include <QObject>
#include "sonardatasourcefile.h"


class SVMClassifier : public QObject {
    Q_OBJECT
public:
    SVMClassifier(QObject *parent = 0);
    ~SVMClassifier();

private:
    CvSVM *svm;
    CvSVMParams svmParam;

    QTimer timer;
    SonarDataSourceFile *file;

public slots:
    void loadSamples(QString path);
    void initSVM();
    void save();
    void load();
    void readSonarFile(QString path);

private slots:
    void timerSlot();
signals:
    void updateUI(SonarReturnData data);
};

#endif // SVMCLASSIFIER_H
