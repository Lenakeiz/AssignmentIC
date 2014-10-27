#pragma once
#define DIRECTINPUT_VERSION 0x0800

#include <guiddef.h>
#include <dinput.h>
#include <dinputd.h>
#include <d3d11.h>

namespace octet{

   struct DI_ENUM_CONTEXT{
      DIJOYCONFIG* preferredJoyCfg;
      bool preferredJoyCfgValid;
   };

   class Joystick
   {
      private:
         //IDirectInput8* directInput;
         LPDIRECTINPUT8 directInput = nullptr;
         //IDirectInputDevice8* joystick;
         LPDIRECTINPUTDEVICE8 joystick = nullptr;
      public:

         Joystick() : directInput(NULL), joystick(NULL)
         {
         }

         ~Joystick()
         {
            //delete directInput;
            //delete joystick;
         }

         //Chuck: Callback function of EnumDevices; called everytime direct input found a device (filtered),
         //diInstance is a pointer from directinput having info about the current device found
         static BOOL CALLBACK EnumJoystickCallback(const DIDEVICEINSTANCE* diInstance, VOID* pContext){

            auto context = reinterpret_cast<Joystick*>(pContext);
            
            HRESULT hr = context->directInput->CreateDevice(diInstance->guidInstance, &(context->joystick), nullptr);
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on interfacing with the controller");
               }
               assert(0);
            }

            return DIENUM_CONTINUE;

         }

         static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* didoi, VOID* pContext){

            auto context = reinterpret_cast<Joystick*>(pContext);

            if (didoi->dwType & DIDFT_AXIS){

               DIPROPRANGE diprg;
               diprg.diph.dwSize = sizeof(DIPROPRANGE);
               diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
               diprg.diph.dwHow = DIPH_BYID;
               diprg.diph.dwObj = didoi->dwType; // Specify the enumerated axis
               diprg.lMin = -100;
               diprg.lMax = +100;

               // Set the range for the axis
               if (FAILED(context->joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
                  return DIENUM_STOP;
               
            }

            return DIENUM_CONTINUE;

         }

         void InitInputDevice(app* ap)
         {

            HWND window = ap->getHWND();
            HINSTANCE inst = GetWindowInstance(window);

            HRESULT hr;
            
            printf("Register with the DirectInput subsystem \n");
            hr = DirectInput8Create(inst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL); //Chuck: passing current windows instance, defined input version (see reference), 

            if (FAILED(hr))
            {
               if (DEBUG_EN){
                  printf("Register with the DirectInput subsystem\n");
               }
               assert(0);
            }

            DIJOYCONFIG PreferredJoyCfg = { 0 };
            DI_ENUM_CONTEXT enumContext;
            enumContext.preferredJoyCfg = &PreferredJoyCfg;
            enumContext.preferredJoyCfgValid = false;
            
            IDirectInputJoyConfig8* pJoyConfig = nullptr;
            hr = directInput->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig); //Chuck: determines whether the DirectInputJoyConfig object supports a particular COM interface, not actually needed
            if (FAILED(hr))
            {
               if (DEBUG_EN){
                  printf("Joystick interface not detected\n");
               }
               assert(0);
            }

            //Chuck: getting info about the joystick 
            PreferredJoyCfg.dwSize = sizeof(PreferredJoyCfg);
            hr = pJoyConfig->GetConfig(0, &PreferredJoyCfg, DIJC_GUIDINSTANCE); //Chuck: joystick number, getting back the guid instance to passe in create device 
            if (SUCCEEDED(hr)){
               enumContext.preferredJoyCfgValid = true;
            }

            //Chuck: Releasing the pointer (safe mode)
            if (pJoyConfig) { (pJoyConfig)->Release(); (pJoyConfig) = nullptr; }
            //DI8DEVCLASS_GAMECTRL
            hr = directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, Joystick::EnumJoystickCallback, (VOID*)this, DIEDFL_ATTACHEDONLY); //Chuck: callback function for any detected devices, passing enumcontext parameter 
            if (FAILED(hr))
            {
               if (DEBUG_EN){
                  printf("Joystick not found\n");
               }
               assert(0);
            }

            //Chuck: check current joystick existence, it avooids calling the set data format before the callback.
            if (!joystick)
            {
               return;
            }

            hr = joystick->SetDataFormat(&c_dfDIJoystick); //Chuck: specify what kind of structure we will have when using ::GetDeviceState
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed to assign data format\n");
               }
               assert(0);
            }               

            if (FAILED(hr = joystick->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_BACKGROUND)))
               assert(0);

            //Chuck: enumerate the joystick objects. In callback we set range for the values.
            hr = joystick->EnumObjects(Joystick::EnumObjectsCallback, (VOID*) this, DIDFT_AXIS);
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on enumerating joystick objects\n");
               }
               assert(0);
            } 

         }

         btVector3 AcquireInputData(){
            
            HRESULT hr;
            DIJOYSTATE js;

            if (!joystick)
               return btVector3(0,0,0);

            //Chuck: Poll the device to read the current state
            hr = joystick->Poll();
            if (FAILED(hr))
            {
               //Chuck: input is interrupted, we aren't tracking any state between polls, so
               // we don't have any special reset that needs to be done. We just re-acquire and try again.
               hr = joystick->Acquire();
               return btVector3(0, 0, 0);
            }

            //Chuck: get the input's device state
            hr = joystick->GetDeviceState(sizeof(DIJOYSTATE), &js);

            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on acquiring device data \n");
               }
               return btVector3(0, 0, 0);
            }

            return btVector3(js.lX, 0, js.lY);
         
         }

         void ShutDown()
         {
            if (joystick != nullptr){
               joystick->Unacquire();
               joystick->Release();
            }

            if (!directInput){
               directInput->Release();
            }
         }

   };
}


