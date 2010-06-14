#ifndef DATALOGHELPER_H
#define DATALOGHELPER_H

#include <QtCore>

class DataLogHelper
{

public:
  static QString getLogDir();

private:
  static QDir path;

};

#endif // DATALOGHELPER_H
