#include <QtGui/QApplication>
#include "trainingwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TrainingWindow w;
    w.show();
    return a.exec();
}
