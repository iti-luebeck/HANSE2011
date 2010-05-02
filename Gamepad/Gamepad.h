#include <QtCore/QCoreApplication>

#include <windows.h>
#include <string.h>
#include <iostream>

using namespace std;

HWND main_window_handle;


#include <initguid.h>
#include <dinput.h> // DirectInput header

LPDIRECTINPUT8 lpdii;                                                       // LPDIRECTINPUT lpdii;
//GUID lpdii;
HINSTANCE hmain;
unsigned short num_joypads = 0; // Number of plugged in joypads
char joynames[8][MAX_PATH]; // Strings of all the names
GUID joyid[8]; // Joypad ID's
GUID joystickGUID;
char joystickNAME[MAX_PATH];
LPDIRECTINPUTDEVICE2 lpdijoystick2;
LPDIRECTINPUTDEVICE8 lpdiid;
LPDIRECTINPUTDEVICE8 lpdijoy   = NULL;    // dinput joystick
LPDIRECTINPUTDEVICE2 joy[8];
DIJOYSTATE joys[8];
DIJOYSTATE joy_state;
DIPROPRANGE joyrange;
bool joy_in[8];
bool joystick_found = false;

typedef struct {
    short x1_offset, y1_offset, x2_offset, y2_offset;
    short x1_direction, y1_direction, x2_direction, y2_direction;
    short x1, y1, x2, y2;
    short old_x1, old_y1, old_x2, old_y2;
    short slider;
    short old_slider;
    bool button[4];
    bool old_button[4];
    bool has_changed;
} gamepadstruct;

gamepadstruct gamepad;

/*
typedef struct
{
     short joy_axis_x,joy_axis_y = 0;
     bool button[36];
}joyplug_state; // Specialized joypad state
joyplug_state joystate;
*/
bool CALLBACK enum_joypads(LPCDIDEVICEINSTANCE lpddi,LPVOID guid_ptr)
{

    joystickGUID = lpddi->guidInstance;

    // copy name into global
     for(int x = 0;x < MAX_PATH;x++)
         joynames[0][x] = lpddi->tszProductName[x];
        //strcpy(joystickNAME, (char *)lpddi->tszProductName);


    return DIENUM_CONTINUE;

}

BOOL CALLBACK DI_Enum_Joysticks(LPCDIDEVICEINSTANCE lpddi, LPVOID guid_ptr)
{
// this function enumerates the joysticks, but
// stops at the first one and returns the
// instance guid of it, so we can create it

*(GUID*)guid_ptr = lpddi->guidInstance;

// copy name into global
//strcpy(joystickNAME, (char *)lpddi->tszProductName);
for(int x = 0;x < MAX_PATH;x++)
    joystickNAME[x] = lpddi->tszProductName[x];

// stop enumeration after one iteration
return(DIENUM_STOP);

} // end DI_Enum_Joysticks

// Creates the joypad objects
bool init_joypads(HINSTANCE hmodule) {


    if(FAILED(DirectInput8Create(hmodule,DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&lpdii,NULL))) {// Create the main object
        std::cout << "Konnte DirectInput nicht initialisieren!" << std::endl;
          return false; // Return failure code
      }

     std::cout << "DInput Joypad Open." << std::endl;
     if(FAILED(lpdii->EnumDevices(DI8DEVCLASS_GAMECTRL,DI_Enum_Joysticks,&joystickGUID,DIEDFL_ATTACHEDONLY))) {
         std::cout << "IDirectInput::EnumDevices fehlgeschlagen!" << std::endl;
          return false; // Return failure code
      }

         if (FAILED(lpdii->CreateDevice(joystickGUID,&lpdijoy,NULL))) {
            std::cout << "Kein einem Gamepad gespielt werden!" << std::endl;
         }

          // Set cooperative level
          if(FAILED(lpdijoy->SetCooperativeLevel(main_window_handle,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
              std::cout << "SetCooperativeLevel fehlgeschlagen!" << std::endl;
               return false;
           }
          std::cout << "SetCooperativeLevel erfolgreich" << std::endl;
          if(FAILED(lpdijoy->SetDataFormat(&c_dfDIJoystick))) {
              std::cout << "SetDataFormat fehlgeschlagen!" << std::endl;
               return false;
           }


          DIPROPRANGE joy_axis_range;

          // first x axis
          joy_axis_range.lMin = -120;
          joy_axis_range.lMax = 120;

          joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE);
          joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          joy_axis_range.diph.dwObj        = DIJOFS_X;
          joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

          lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

          // now y-axis
          joy_axis_range.lMin = -120;
          joy_axis_range.lMax = 120;

          joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE);
          joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          joy_axis_range.diph.dwObj        = DIJOFS_Y;
          joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

          lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

          // now y-axis
          joy_axis_range.lMin = -50;
          joy_axis_range.lMax = 50;

          joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE);
          joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          joy_axis_range.diph.dwObj        = DIJOFS_Z;
          joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

          lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

          // now y-axis
          joy_axis_range.lMin = -50;
          joy_axis_range.lMax = 50;

          joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE);
          joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
          joy_axis_range.diph.dwObj        = DIJOFS_RZ;
          joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

          lpdijoy->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

          gamepad.x1_direction = 1;
          gamepad.y1_direction = -1;
          gamepad.x2_direction = 1;
          gamepad.y2_direction = -1;

          gamepad.x1_offset = 0;
          gamepad.y1_offset = 0;
          gamepad.x2_offset = 0;
          gamepad.y2_offset = 0;

          if (lpdijoy->Acquire()!=DI_OK) {
              cout << "Acquire nicht ok" << endl;
              return false;
          }

          joystick_found = true;
     return true; // Return success code
}
void free_joypads()
{
     // Call this on game exit
     for(int x = 0;x <8;x++)
     {
          if(joy[x])
          {
               joy[x]->Unacquire();
               joy[x]->Release();
          }
     }
     if(lpdii)
          lpdii->Release();
}
// Returns number of joypads
int joypads()
{
     return num_joypads;
}
// Returns a joypad vendor name (returns the pointer to string)
char *str_joy(int index)
{
     return &joynames[index][0];
}

char *joystickName() {
    return &joystickNAME[0];
}

// Gathers the joypad states
void get_state() {
    lpdijoy->Poll();

    if(FAILED(lpdijoy->GetDeviceState(sizeof(DIJOYSTATE),(LPVOID)&joy_state))) {
        std::cout << "lpdijoy->GetDeviceState failed!" << std::endl;
    }

    gamepad.old_x1 = gamepad.x1;
    gamepad.old_y1 = gamepad.y1;
    gamepad.old_x2 = gamepad.x2;
    gamepad.old_y2 = gamepad.y2;
    for (int i=0; i<4; i++) gamepad.old_button[i] = gamepad.button[i];

    gamepad.x1 = (joy_state.lX - gamepad.x1_offset)*gamepad.x1_direction;
    gamepad.y1 = (joy_state.lY - gamepad.y1_offset)*gamepad.y1_direction;
    gamepad.x2 = (joy_state.lRz - gamepad.x2_offset)*gamepad.x2_direction;
    gamepad.y2 = (joy_state.lZ - gamepad.y2_offset)*gamepad.y2_direction;

    for (int i=0; i<4; i++) gamepad.button[i] = (joy_state.rgbButtons[i] > 0 ) ? 1 : 0;

    gamepad.old_slider = gamepad.slider;
    gamepad.slider = joy_state.rglSlider[0];

    gamepad.has_changed = false;

    if ( (gamepad.old_x1!=gamepad.x1) || \
         (gamepad.old_y1!=gamepad.y1) || \
         (gamepad.old_x2!=gamepad.x2) || \
         (gamepad.old_y2!=gamepad.y2) || \
         (gamepad.old_slider!=gamepad.slider)) gamepad.has_changed = true;

    for (int i=0; i<4; i++) {
        if(gamepad.old_button[i]!=gamepad.button[i]) gamepad.has_changed = true;
    }
}
