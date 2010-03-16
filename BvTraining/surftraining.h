#ifndef SURFTRAINING_H
#define SURFTRAINING_H

#include <QString>
#include <QList>

class SurfTraining
{
public:
    SurfTraining();

    void train(QList<int> frameList, QString videoFile);
    void test(QString videoFile);
    void save(QString saveFile);
    void load(QString loadFile);
};

#endif // SURFTRAINING_H
