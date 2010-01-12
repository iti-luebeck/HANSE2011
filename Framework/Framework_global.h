#ifndef FRAMEWORK_GLOBAL_H
#define FRAMEWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FRAMEWORK_LIBRARY)
#  define FRAMEWORKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FRAMEWORKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FRAMEWORK_GLOBAL_H
