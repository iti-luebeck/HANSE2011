#ifndef MODULESGRAPH_H
#define MODULESGRAPH_H

#include <QtCore>
#include "robotmodule.h"
#include <log4qt/logger.h>

class RobotModule;

class ModulesGraph : public QObject
{
public:
    ModulesGraph();

    /**
      * Return list of all modules
      */
    QList<RobotModule*> getModules();

    /**
      * Terminates all active modules in reverse creation order.
      */
    void HastaLaVista(void);

private:
    /**
      * List of modules (won't change after init)
      */
    QList<RobotModule*> modules;

    /**
      * Logger
      */
    Log4Qt::Logger* logger;

public:
    /**
      * Create all modules
      */
    void build(void);

};

#endif // MODULESGRAPH_H
