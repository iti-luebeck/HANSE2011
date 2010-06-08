#ifndef HANSEAPP_H
#define HANSEAPP_H

#include <QApplication>

class HanseApp : public QApplication
{
public:
    HanseApp(int, char**);

    bool notify(QObject *, QEvent *);
};

#endif // HANSEAPP_H
