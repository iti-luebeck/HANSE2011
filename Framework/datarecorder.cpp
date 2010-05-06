#include "datarecorder.h"

DataRecorder::DataRecorder(RobotModule& module)
    : module(module)
{
    logger = Log4Qt::Logger::logger("DataRecorder");

    connect(&module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(newDataReceived(RobotModule*)));

    path = "logs/"+QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm");
    QDir(".").mkpath(path);
    file = new QFile(path+"/"+module.getId()+".csv");
}

void DataRecorder::open()
{
    QStringList dataKeysNew = module.getData().keys();
    dataKeysNew.sort();

    QStringList settingsKeysNew = module.getSettings().allKeys();
    settingsKeysNew.sort();

    // TODO: compare lists
    bool listsChanged = false;

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
        }

        dataKeys = module.getData().keys();
        dataKeys.sort();

        settingsKeys = module.getSettings().allKeys();
        settingsKeys.sort();
    }

}

void DataRecorder::newDataReceived(RobotModule *module)
{
    open();

    logger->debug("bla 1");

    if (!stream) {
        return;
    }

    logger->debug("bla 3");

    foreach (QString key, settingsKeys) {
        *stream << module->getSettings().value(key).toString() << ",";
    }

    foreach (QString key, dataKeys) {
        *stream << module->getData().value(key).toString() << ",";
    }
    *stream << "das wars\n";
    logger->debug("bla 4");

}

void DataRecorder::close()
{
    stream->flush();
    logger->debug("bla6");
    file->close();
}
