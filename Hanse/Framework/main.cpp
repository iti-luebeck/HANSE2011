#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QSettings>

int main(int argc, char *argv[])
{
    // Store config in current working directory and store
    // it as plain ini-files, not in this mysteeerious windows registry
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, ".");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication a(argc, argv);

    a.setApplicationName("Hanse");
    a.setOrganizationName("ITI");

    MainWindow w;
    w.show();
    return a.exec();
}
