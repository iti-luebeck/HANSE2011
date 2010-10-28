#include "datarecorder.h"
#include <Framework/dataloghelper.h>

DataRecorder::DataRecorder(RobotModule& module)
    : module(module)
{
    logger = Log4Qt::Logger::logger("DataRecorder");

    connect(&module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(newDataReceived(RobotModule*)));
    connect(&module, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(newDataReceived(RobotModule*)));

    file = new QFile(DataLogHelper::getLogDir()+module.getId()+".csv");

    fileCount = 0;
}

bool DataRecorder::isChanged(QStringList a, QStringList b)
{
    if (a.size() != b.size())
        return true;

    for(int i=0; i<a.size(); i++) {
        if (a[i] != b[i])
            return true;
    }
    return false;
}

void DataRecorder::open()
{
    QStringList dataKeysNew = module.getDataCopy().keys();
    dataKeysNew.sort();

    QStringList settingsKeysNew = module.getSettingKeys();
    settingsKeysNew.sort();
    settingsKeysNew.removeOne("enableLogging");

    bool listsChanged = isChanged(dataKeys, dataKeysNew) || isChanged(settingsKeys, settingsKeysNew);

    if (file->isOpen() && listsChanged) {
        file->close();
        fileCount++;
        file = new QFile(DataLogHelper::getLogDir()+module.getId()+"_"+QString::number(fileCount)+".csv");
        stream = NULL;
    }

    if (!file->isOpen()) {
        if (file->exists())
            logger->error("File "+ file->fileName()+ " already exists, although it shouldn't!");

        // this file should always be a new one. but be sure that no data
        // is lost, just append everything.
        if (file->open(QFile::WriteOnly | QIODevice::Append)) {
            stream = new QTextStream(file);
        } else {
            logger->error("Could not open file "+file->fileName());
            return;
        }

        dataKeys = module.getDataCopy().keys();
        dataKeys.sort();

        settingsKeys = module.getSettingKeys();
        settingsKeys.sort();
        settingsKeys.removeOne("enableLogging");

        *stream << ";time,healthStatus,healthErrorMsg";
        foreach (QString key, settingsKeys) {
            *stream << "," << key;
        }

        foreach (QString key, dataKeys) {
            *stream << "," << key;
        }
        *stream << "\r\n";

        stream->flush();
    }

}

void DataRecorder::newDataReceived(RobotModule *module)
{
    if (!module->getSettingsValue("enableLogging").toBool()) {
        return;
    }

    open();

    if (!file->isOpen()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();

    *stream << now.toTime_t() << "." << now.toString("z") << ",";
    *stream << module->getHealthStatus().isHealthOk();

    // TODO: write health status to different logfile
//    *stream << "\"" << module->getHealthStatus().getLastError() << "\"";
    QMap<QString,QVariant> settingsMap;
    settingsMap = module->getSettingsCopy();

    foreach (QString key, settingsKeys) {
        if (key == "enableLogging")
            continue;

        QVariant value =settingsMap.value(key);
//        QVariant value = module->getSettingsValue(key);
        if (value.convert(QVariant::Double)) {
            *stream << "," << value.toDouble();
            continue;
        }
        value = settingsMap.value(key);
//        value = module->getSettingsValue(key);
        if (value.convert(QVariant::Bool)) {
            *stream << "," << value.toBool();
            continue;
        }
//        value = module->getSettings().value(key);
//        if (value.convert(QVariant::String)) {
//            *stream << ",\"" << value.toString() << "\"";
//        }
    }

    foreach (QString key, dataKeys) {
        QVariant value = module->getDataValue(key);
        if (value.convert(QVariant::Double)) {
            *stream << "," << value.toDouble();
            continue;
        }
        value = module->getDataValue(key);
        if (value.convert(QVariant::String) && value.toString() != "false" && value.toString() != "true") {
            *stream << ",\"" << value.toString() << "\"";
            continue;
        }
        value = module->getDataValue(key);
        if (value.convert(QVariant::Bool)) {
            *stream << "," << value.toBool();
            continue;
        }
    }

    *stream << "\r\n";

    // the app may crash...
    stream->flush();
}

void DataRecorder::close()
{
    logger->debug("Closing data log file for "+module.getId());
    file->close();
}
