#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QtCore>
#include "client.h"
#include <windows.h>

class Joystick : public QObject
{
    Q_OBJECT

public:
    Joystick(HINSTANCE hInstance);

private:
    QTimer timer;
    Client client;

private slots:
    void timerElapsed();
};

#endif // JOYSTICK_H
