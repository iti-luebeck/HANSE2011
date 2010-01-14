#ifndef MODULESGRAPH_H
#define MODULESGRAPH_H

#include <QObject>
#include <robotmodule.h>
#include <log4qt/logger.h>

class ModulesGraph : public QObject
{
public:
    ModulesGraph();

    /**
      * Return list of all modules
      */
    QList<RobotModule*> getModules();

private:
    /**
      * List of modules (won't change after init)
      */
    QList<RobotModule*> modules;

    /**
      * Logger
      */
    Log4Qt::Logger* logger;

    /**
      * Create all modules
      */
    void build(void);

};

#endif // MODULESGRAPH_H
