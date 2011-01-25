#include "echodatacsvrecorder.h"
#include <Framework/dataloghelper.h>
#include "echodatarecorder.h"
#include "echoreturndata.h"
#include <QtCore>
#include "module_echosounder.h"

EchoDataCSVRecorder::EchoDataCSVRecorder(Module_EchoSounder& echo)
    : EchoDataRecorder(echo)
{
    file = NULL;
    stream = NULL;
}

void EchoDataCSVRecorder::start(){
    if(file != NULL){
        stop();
    }
    file = new QFile(DataLogHelper::getLogDir()+"echolog.csv");
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        stream = new QTextStream(file);
    } else {
        logger()->error("Could not open file "+file->fileName());
    }
}

void EchoDataCSVRecorder::stop()
{
    if (file != NULL) {
        file->close();
        delete file;
        file = NULL;
    }

    if (stream != NULL) {
        delete stream;
        stream = NULL;
    }
}

void EchoDataCSVRecorder::store(const EchoReturnData &data){
    if(!stream || !file){
        logger()->error("Cannot record. Stream not open");
        return;
    }

    time_t t = data.switchCommand.time.toTime_t();

    *stream << data.getRange() << "," << data.switchCommand.startGain;
        *stream << "," << t;
    for(int i = 0; i<data.getEchoData().length(); i++){
        *stream << "," << (int)data.getEchoData().at(i);
    }
    *stream << "\n";
    stream->flush();
}
