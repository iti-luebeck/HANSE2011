#ifndef MODULE_UID_GLOBAL_H
#define MODULE_UID_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MODULE_UID_LIBRARY)
#  define MODULE_UIDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MODULE_UIDSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MODULE_UID_GLOBAL_H
