#include "datarecorder.h"

DataRecorder::DataRecorder(RobotModule& module)
    : module(module)
{
    logger = Log4Qt::Logger::logger("DataRecorder");

    connect(&module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(newDataReceived(RobotModule*)));
    connect(&module, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(newDataReceived(RobotModule*)));

    path = "logs/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm");
    QDir(".").mkpath(path);
    file = new QFile(path+"/"+module.getId()+".csv");

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
    QStringList dataKeysNew = module.getData().keys();
    dataKeysNew.sort();

    QStringList settingsKeysNew = module.getSettings().allKeys();
    settingsKeysNew.sort();
    settingsKeysNew.removeOne("enableLogging");

    bool listsChanged = isChanged(dataKeys, dataKeysNew) || isChanged(settingsKeys, settingsKeysNew);

    if (file->isOpen() && listsChanged) {
        file->close();
        fileCount++;
        file = new QFile(path+"/"+module.getId()+"_"+QString::number(fileCount)+".csv");
        stream = NULL;
    }

    if (!file->isOpen()) {
        if (file->open(QFile::WriteOnly | QFile::Truncate)) {
            stream = new QTextStream(file);
        } else {
            logger->error("Could not open file "+file->fileName());
            return;
        }

        dataKeys = module.getData().keys();
        dataKeys.sort();

        settingsKeys = module.getSettings().allKeys();
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
    if (!module->getSettings().value("enableLogging").toBool()) {
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

    foreach (QString key, settingsKeys) {
        if (key == "enableLogging")
            continue;

        QVariant value = module->getSettings().value(key);
        if (value.convert(QVariant::Double)) {
            *stream << "," << value.toDouble();
            continue;
        }
        value = module->getSettings().value(key);
        if (value.convert(QVariant::Bool)) {
            *stream << "," << value.toBool();
        }
    }

    foreach (QString key, dataKeys) {
        QVariant value = module->getData().value(key);
        if (value.convert(QVariant::Double)) {
            *stream << "," << value.toDouble();
            continue;
        }
        value = module->getData().value(key);
        if (value.convert(QVariant::Bool)) {
            *stream << "," << value.toBool();
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
