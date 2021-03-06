#include "hanseapp.h"
#include <QMessageBox>

#include <opencv/cv.h>

HanseApp::HanseApp(int argc, char** argv)
    : QApplication(argc, argv)
{
    setOrganizationName("HanseCfg");
}


bool HanseApp::notify(QObject *o, QEvent *e)
{
    try
    {
            return QApplication::notify(o, e);
    }
    catch (cv::Exception e)
    {
        return true;
    }
    catch(...)
    {
        return true;
    }
}
