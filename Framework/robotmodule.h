#ifndef ROBOTMODULE_H
#define ROBOTMODULE_H

#include <QObject>
#include <QString>
#include <QSettings>
#include "Framework_global.h"



class FRAMEWORKSHARED_EXPORT RobotModule : public QObject
{
    Q_OBJECT

public:

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

signals:
    /**
      * Sends a signal when this module gets activated/deactivated
      */
    void isEnabled(bool value);

public slots:

    /**
      * Resets the runtime, non-persistent state of the module back to
      * its initial state.
      * (e.g. "stop thrusters", "reset state machine back to starting point")
      */
    virtual void reset() = 0;

    /**
      * Enabled/disables this module
      */
    void enabled(bool value);

protected:
    /**
      * All persistent configuration of the module is stored in here
      */
    QSettings settings;

private:
    /**
      * ID of the module. must be unique across all robot instances. won't change at runtime.
      */
    const QString id;

};

#endif // ROBOTMODULE_H
