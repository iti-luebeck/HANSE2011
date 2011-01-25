#include "echodatarecorder.h"
#include "echoreturndata.h"
#include <QtCore>
#include "module_echosounder.h"

EchoDataRecorder::EchoDataRecorder(Module_EchoSounder& s)
: sounder(s)
{
    //logger = Log4Qt::Logger::logger("EchoRecorder");

    connect(&s, SIGNAL(newEchoData(EchoReturnData)), this, SLOT(newData(EchoReturnData)));

}

void EchoDataRecorder::newData(const EchoReturnData data)
{
    store(data);
}

