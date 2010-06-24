#include <QtCore/QCoreApplication>
#include <QtTest/QTest>
#include <QtCore>
#include <windows.h>
#include "joystick.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
    int  argc = 1;
    char *argv[1];

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, ".");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication a(argc, argv);
    a.setApplicationName("HanseGamepad");
    a.setOrganizationName("ITI");

    new Joystick(hInstance);

    return a.exec();
}
