#ifndef MODULESGRAPH_H
#define MODULESGRAPH_H

#include <QObject>
#include <robotmodule.h>

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
      * Create all modules
      */
    void build(void);

};

#endif // MODULESGRAPH_H
