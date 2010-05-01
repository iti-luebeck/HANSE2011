#include <QtCore/QCoreApplication>
#include <iostream>
#include "module_uid.h"

using namespace std;


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Module_UID *uid = new Module_UID("uid1");

    if ( !(uid->getUID()->UID_Available()) ) {
        cerr << "No UID found" << endl;
    }
    else {
        cout << "Identification: " <<uid->getUID()->UID_Identify().toStdString() << endl;
        cout << "Revision: " << uid->getUID()->UID_Revision().toStdString() << endl;

        unsigned char result[2];
        if (uid->getUID()->UID_ADC(0b00001010,result)) {
            cout << result[0] << endl;
            cout << result[1] << endl;
        }
        else {
            cerr << "could not read from ADC"<< endl;
        }
    }

    return a.exec();
}
