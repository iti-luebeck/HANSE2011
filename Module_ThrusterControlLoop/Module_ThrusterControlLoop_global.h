#ifndef MODULE_THRUSTERCONTROLLOOP_GLOBAL_H
#define MODULE_THRUSTERCONTROLLOOP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MODULE_THRUSTERCONTROLLOOP_LIBRARY)
#  define MODULE_THRUSTERCONTROLLOOPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MODULE_THRUSTERCONTROLLOOPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MODULE_THRUSTERCONTROLLOOP_GLOBAL_H