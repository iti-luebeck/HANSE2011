#ifndef BLOBTRAINING_H
#define BLOBTRAINING_H

#include <QString>
#include <QList>
#include "imageprocessor.h"
#include "SVMClassifier.h"

class BlobTraining
{
private:
    ImageProcessor ip;
    SVMClassifier svm;

public:
    BlobTraining();

    void train(QList<int> frameList, QString videoFile);
    void test(QString videoFile);
    void save(QString saveFile);
    void load(QString loadFile);
};

#endif // BLOBTRAINING_H
