#pragma once
//#define STRICT
#define DIRECTINPUT_VERSION 0x0800
//#define _CRT_SECURE_NO_DEPRECATE
//#ifndef _WIN32_DCOM
//#define _WIN32_DCOM
//#endif
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
         IDirectInput8* directInput;
         IDirectInputDevice8* joystick;
      public:

         Joystick() : directInput(NULL), joystick(NULL)
         {
         }

         ~Joystick()
         {
            delete directInput;
            delete joystick;
         }

         //Chuck: Callback function of EnumDevices; called everytime direct input found a device (filtered)
         static BOOL CALLBACK EnumJoystickCallback(const DIDEVICEINSTANCE* diInstance, VOID* pContext){

            auto pEnumContext = reinterpret_cast<Joystick*>(pContext);
            
            HRESULT hr = pEnumContext->directInput->CreateDevice(diInstance->guidInstance, &(pEnumContext->joystick), nullptr);
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on interfacing with the controller");
               }
               assert(0);
            }

            return DIENUM_CONTINUE;

         }

         void InitInputDevice(app* ap)
         {

            HWND window = ap->getHWND();
            HINSTANCE inst = GetWindowInstance(window);

            HRESULT hr;
            
            printf("Register with the DirectInput subsystem");
            hr = DirectInput8Create(inst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL); //Chuck: passing current windows instance, defined input version (see reference), 

            if (FAILED(hr))
            {
               if (DEBUG_EN){
                  printf("Register with the DirectInput subsystem");
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
                  printf("Joystick interface not detected");
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
            hr = directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, Joystick::EnumJoystickCallback, this, DIEDFL_ATTACHEDONLY); //Chuck: callback function for any detected devices, passing enumcontext parameter 
            if (FAILED(hr))
            {
               if (DEBUG_EN){
                  printf("Joystick not found");
               }
               assert(0);
            }

            hr = joystick->SetDataFormat(&c_dfDIJoystick2); //Chuck: specify what kind of structure we will have when using ::GetDeviceState
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed to assign data format");
               }
               assert(0);
            }               

            //Chuck: need to move on the callback function
            hr = directInput->CreateDevice(GUID_Joystick, &joystick, NULL);
            if (FAILED(hr))
            {
               assert(0);
            }

            //return dI;
         }

   };
}


