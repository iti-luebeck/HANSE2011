#include "hanseapp.h"
#include "mainwindow.h"
#include <QSettings>
#include <Module_Navigation/waypoint.h>

#ifdef OS_UNIX
#include <sys/prctl.h>
#endif

int main(int argc, char *argv[])
{
    // Types must be registered BEFORE the settings are opened,
    // otherwise custom types will be invalid.
    qRegisterMetaType< QMap<QString,Waypoint> >("QMap<QString,Waypoint>");
    qRegisterMetaType<Waypoint>("Waypoint");
    qRegisterMetaTypeStreamOperators<Waypoint>("Waypoint");

    // Store config in current working directory and store
    // it as plain ini-files, not in this mysteeerious windows registry
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, ".");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    qsrand(42);

    QApplication a(argc,argv);
    a.setApplicationName("Hanse");
    a.setOrganizationName("ITI");

#ifdef OS_UNIX
    prctl(PR_SET_NAME,"Hanse GUI");
#endif

    MainWindow w;
    w.show();
    return a.exec();
}
