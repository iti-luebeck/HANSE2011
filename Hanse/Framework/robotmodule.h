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
  * Each robot module may add additional methods.
  *
  * This class is meant to be subclassed.
  *
  * IMPORTANT: This class and all of its methods must be thread-safe!
  *            Authors of subclasses must take care of this as well!
  */
class RobotModule : public QThread
{
    Q_OBJECT

public:

    /**
      * Create a new Robot Module with the given id.
      */
    RobotModule(QString id);

    /**
      * id of the instance. is unique under all instances of all loaded modules
      * must not contain any slashes or backslashes (thread safe)
      */
    QString getId();

    /**
      * this string will be seen on the tab representing this module (thread safe)
      *
      * the default implementation just uses the Id
      */
    virtual QString getTabName();

    /**
      * Create the widget for viewing and configuring stuff
      *
      * (the exact details are up  to the module programmer)
      *
      * must be thread-safe as this will be called from the UI thread!
      */
    virtual QWidget* createView(QWidget* parent) = 0;

    /**
      * Return all modules this module depends on.
      *
      * although not currently used anywhere, it must be implemented thread-safe,
      */
    virtual QList<RobotModule*> getDependencies() = 0;

    /**
      * Returns true if this module is currently enabled (thread safe)
      */
    bool isEnabled();

    /**
      * Return true if the module has been initialized (thread safe)
      */
    bool isInitialized();

    /**
      * A copy of the settings of this module using dataLockerMutex (thread safe)
      */
    const QMap<QString,QVariant> getSettingsCopy();

    /**
      * The setting keys of this module using dataLockerMutex (thread safe)
      */
    QStringList getSettingKeys();

    /**
      * returns value for given key from local settings map using dataLockerMutex (thread safe)
      */
    const QVariant getSettingsValue(const QString key, const QVariant defaultValue);
    const QVariant getSettingsValue(const QString key);

    /**
      * adds given value for given key in settings map using dataLockerMutex (thread safe)
      */
    void setSettingsValue(QString key, const QVariant value);


    /**
      * Returns a copy of the data store using dataLockerMutex (thread safe).
      *
      * Only the owning module itself is allowed to modify it.
      */
    const QMap<QString,QVariant> getDataCopy();

    /**
      * returns value for given key from data map using dataLockerMutex (thread safe)
      */
    const QVariant getDataValue(QString key);

    /**
     * adds given value for given key in data map using dataLockerMutex (thread safe)
     */
    void addData(QString key, const QVariant value);

    /**
      * Returns the current health status of this module (thread safe)
      */
    HealthStatus getHealthStatus();

    /**
      * stops the module from the outside. (thread safe)
      *
      * waits until the module-thread is shutdown
      */
    virtual bool shutdown();

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
      * Enable/disable this module. (thread safe)
      */
    void setEnabled(bool value);

protected:


    /**
      * Overwrite run
      *
      */
    void run();

    /**
      * mutex to lock access to data or settings map. this is the main mutex of the module.
      * the module should protect all of its datastructures using this mutex.
      */
    QMutex dataLockerMutex;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    /**
      * Sets the config value of "key" to "value" if it is not already
      * set using dataLockerMutex (thread safe).
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
      * sleep for the given amount of milliseconds
      */
    virtual void msleep(int millies);

    /**
      * init module.
      *
      * this method is called only once, right after the constructor.,
      * all initialisation should be performed in this method.
      * This method is called from the module's own thread.
      */
    virtual void init() = 0;

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
      *
      * This method is called from the module's own thread.
      */
    virtual void terminate();


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

    /**
      * Resets the runtime, non-persistent state of the module back to
      * its initial state.
      * (e.g. "stop thrusters", "reset state machine back to starting point")
      *
      * This method is not thread safe and can therefor only be called as a SLOT
      */
    virtual void reset();

private:

    /**
      * Central data store for this module. Everything stored in this Map is
      * automatically displayed in the GUI, and will also be recorded to file.
      */
    QMap<QString,QVariant> data;


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
    QMutex healthStatusMutex;

    DataRecorder *recorder;

    /**
      * indicated if the module returned from init(). Meaning it is initialized
      */
    bool initialized;

};

#endif // ROBOTMODULE_H
