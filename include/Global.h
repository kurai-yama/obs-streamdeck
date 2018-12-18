#ifndef OBSPLUGINSTREAMDECK_GLOBAL_H
#define OBSPLUGINSTREAMDECK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(OBSPLUGINSTREAMDECK_LIBRARY)
#  define OBSPLUGINSTREAMDECKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define OBSPLUGINSTREAMDECKSHARED_EXPORT Q_DECL_IMPORT
#endif

#define LOG_STREAMDECK_CLIENT 0xbbf5ff
#define LOG_STREAMDECK 0xcaff9e
#define LOG_STREAMDECK_MANAGER 0x7752ff

#endif // OBSPLUGINSTREAMDECK_GLOBAL_H