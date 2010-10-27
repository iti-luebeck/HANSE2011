#ifndef ROBOTMODULE_MT_H
#define ROBOTMODULE_MT_H

#include <Framework/robotmodule.h>

class DataRecorder;

class RobotModule_MT : public RobotModule
{
    Q_OBJECT

public:

    /**
      * Create a new Robot Module with the given id.
      */
    RobotModule_MT(QString id);

    /**
      * return a copy of local data Map using datalockermutex
      *
      */
      virtual const QMap<QString,QVariant> getData();

        /**
          * returns value for given key from data map using datalockermutex
          */
        QVariant getDataValue(QString key);

        /**
          * returns
          */
        virtual QSettings& getSettings();

        /**
          * returns value for given key from local settings map using settingsMutex
          */
        const QVariant getSettingsValue(const QString key, const QVariant defValue);
        const QVariant getSettingsValue(const QString key);




//signals:


public slots:
        /**
         * adds given value for given key in data map using datalockermutex
         */
        void addData(QString key, const QVariant value);

        /**
          * adds given value for given key in settings map using settingsMutex
          */
        void setSettingsValue(QString key, const QVariant value);

        void setDefaultValue(const QString &key, const QVariant &value);



        void getDataValue(const QString key,  QVariant &data);
        void getSettingsValueSl(const QString key, QVariant &value);
        void getSettingsValueSl(const QString key,const QVariant defValue, QVariant &value);

protected:

    /**
      * sleep for the given amount of milliseconds
      */
    void msleep(int millies);

    QMutex dataLockerMutex;
//    QMutex settingsMutex;
//protected slots:


private:

    class MyModuleThread: public QThread
    {
    public:
        static void msleep(int millies);
        void run();
//        int getID();
    };


    MyModuleThread moduleThread;




};

#endif // RobotModule_MT_H
