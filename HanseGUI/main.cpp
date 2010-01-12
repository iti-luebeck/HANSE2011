#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationDomain("iti.uni-luebeck.de");
    a.setApplicationName("Hanse");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("ITI");

    // Store config in current working directory and store
    // it as plain ini-files, not in this mysteeerious windows registry
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, ".");
    MainWindow w;
    w.show();
    return a.exec();
}
