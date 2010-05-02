#include <QtCore/QCoreApplication>
#include <QtTest/QTest>
#include <windows.h>
#include "gamepad.h"
#include "client.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
  int  argc = 1;
  char *argv[1];

  QCoreApplication a(argc, argv);

  Client client(1234);
  char message[9];
  message[0] = 9;

  short pos;

  if (!(init_joypads(hInstance))) std::cout << "Fehler bei Joystick init" << std::endl;
  if (joystick_found) {


    while(1) {
        get_state();


        if (gamepad.has_changed) {
            cout << gamepad.x1 << " " << gamepad.y1 << " " << gamepad.x2 << " " << gamepad.y2 << " ";
            for (int i=0; i<4; i++) cout << ((gamepad.button[i] > 0 ) ?  "1 " : "0 ");
            cout << endl;
            pos = 1;
            message[pos++] = gamepad.button[0];
            message[pos++] = gamepad.x1;
            message[pos++] = gamepad.y1;
            message[pos++] = gamepad.x2;
            message[pos++] = gamepad.y2;
            client.sendMessage(message);
        }
        QTest::qWait(50);
    }

  }

  a.exec();


  free(argv[0]);
   return 0;

}
