#ifndef ROBOTMODULE_H
#define ROBOTMODULE_H

#include <QtCore>
#include <QMap>
#include <log4qt/logger.h>
#include "healthstatus.h"

class DataRecorder;

/**
  * Abstract super class of every robot module.
  *
  * This class is meant to be subclassed.
  */
class RobotModule : public QObject
{
    Q_OBJECT

public:

    /**
      * Create a new Robot Module with the given id.
      */
    RobotModule(QString id);

    /**
      * id of the instance. is unique under all instances of all loaded modules
      * must not contain any slashes or backslashes
      */
    QString getId();

    /**
      * this string will be seen on the tab representing this module
      *
      * the default implementation just uses the Id
      */
    virtual QString getTabName();

    /**
      * Create the widget for viewing and configuring stuff
      * (the exact details are up  to the module programmer)
      */
    virtual QWidget* createView(QWidget* parent) = 0;

    /**
      * Return all modules this module depends on.
      */
    virtual QList<RobotModule*> getDependencies() = 0;

    /**
      * Returns true if this module is currently enabled
      */
    bool isEnabled();

    /**
      * The settings of this module
      */
    QSettings& getSettings();

    /**
      * Returns the data store.
      *
      * Only the owning module itself is allowed to modify it.
      */
    const QMap<QString,QVariant> getData();

    /**
      * Returns the current health status of this module
      */
    HealthStatus getHealthStatus();

signals:
    /**
      * Sends a signal when this module gets activated/deactivated
      */
    void enabled(bool value);

    /**
      * Signals that the health status of the module has changed.
      *
      * this signals indicated that the HealthStatus has changed in any way.
      */
    void healthStatusChanged(RobotModule *module);

    /**
      * Signals a change in the "data" map.
      *
      * After something in the map has been changed, this signal must be emitted.
      *
      * Note: The RobotModule may aggregate this signal, i.e. emit it only after a
      * couple of different meassurements have been performed.
      */
    void dataChanged(RobotModule *module);

public slots:

    /**
      * Resets the runtime, non-persistent state of the module back to
      * its initial state.
      * (e.g. "stop thrusters", "reset state machine back to starting point")
      */
    virtual void reset();

    /**
      * Enable/disable this module
      */
    void setEnabled(bool value);

    /**
      * This method is called when the module is destroyed (in other words:
      * the program terminates)
      *
      * The module should clean its mess up in this method; close/flush open
      * files, close serial ports and of cource terminate any threads it started.
      *
      * This method is called exactly once during the lifetime of a module.
      * The modules are terminated in reversed order of creation: It is guaranteed
      * that all dependencies of a module A remain alive until this module A is
      * terminated.
      */
    virtual void terminate();

protected:
    /**
      * All persistent configuration of the module is stored in here
      */
    QSettings settings;

    /**
      * Central data store for this module. Everything stored in this Map is
      * automatically displayed in the GUI, and will also be recorded to file.
      */
    QMap<QString,QVariant> data;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    /**
      * Sets the config value of "key" to "value" if it is not already set.
      */
    void setDefaultValue(const QString &key, const QVariant &value);

    /**
      * Mark this module as faulty. The error message will be logged and displayed
      * to the user. The error counter will be incremented.
      */
    void setHealthToSick(QString errorMsg);

    /**
      * Return health status to healthy, i.e. the module has no known problems.
      *
      */
    void setHealthToOk();

    /**
      * sleep for the given amount of time
      */
    void sleep(int millies);

protected slots:
    /**
      * Do a health check.
      *
      * This method is called every once in a while (e.g. one second) and allows you
      * to perform some checks to see if your module is working properly.
      *
      * The default implementation will do no checks.
      */
    virtual void doHealthCheck();

private:
    class MyQThread: public QThread
    {
    public:
        static void sleep(int millies);
    };

    /**
      * ID of the module. must be unique across all robot instances. won't change at runtime.
      */
    const QString id;

    /**
      * Current health status
      */
    HealthStatus healthStatus;

    /**
      * Timer to perform regular health checks.
      */
    QTimer healthCheckTimer;

    DataRecorder *recorder;

};

#endif // ROBOTMODULE_H
