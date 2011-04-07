#include <QtGui/QApplication>
#include "sltrainingui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SLTrainingUI w;
    w.show();

    return a.exec();
}
