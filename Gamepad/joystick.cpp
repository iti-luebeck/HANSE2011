#include "joystick.h"

#include <QtCore/QCoreApplication>
#include <QtTest/QTest>
#include <QtCore>
#include <windows.h>
#include "gamepad.h"

Joystick::Joystick(HINSTANCE hInstance)
{
    if (!(init_joypads(hInstance))) {
        std::cout << "Fehler bei Joystick init" << std::endl;
        while (1) QTest::qWait(1000);
    }
    if (!joystick_found) {
        std::cout << "No joystick found." << std::endl;
        while (1) QTest::qWait(1000);
    }

    timer.start(50);
}

void Joystick::timerElapsed()
{

    get_state();

    if (gamepad.has_changed) {
        cout << gamepad.x1 << " " << gamepad.y1 << " " << gamepad.x2 << " " << gamepad.y2 << " ";
        for (int i=0; i<4; i++) cout << ((gamepad.button[i] > 0 ) ?  "1 " : "0 ");
        cout << endl;
        client.sendMessage(gamepad.x1, gamepad.y1, gamepad.x2, gamepad.button[1]);
    }
}
