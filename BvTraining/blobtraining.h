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
    QList<int> frameList;
    SVMClassifier svm;

public:
    BlobTraining();

    void select(QString videoFile);
    void train(QString videoFile);
    void test(QString videoFile);
    void save(QString saveFile);
    void load(QString loadFile);
};

#endif // BLOBTRAINING_H
