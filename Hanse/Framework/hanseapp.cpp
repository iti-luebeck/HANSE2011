#include "hanseapp.h"
#include <QMessageBox>

#include <opencv/cv.h>

HanseApp::HanseApp()
    : QApplication(0,0)
{
    setApplicationName("Hanse");
    setOrganizationName("ITI");
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
