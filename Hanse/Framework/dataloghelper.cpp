#include "dataloghelper.h"

QDir DataLogHelper::path = QDir::currentPath()+"/logs/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm_zzz")+"/";

QString DataLogHelper::getLogDir()
{
    path.makeAbsolute();
    path.mkpath(path.path());
    return path.path()+"/";
}
