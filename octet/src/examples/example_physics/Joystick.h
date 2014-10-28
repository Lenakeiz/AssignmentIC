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

   class Joystick : public resource
   {

      private:
         
         LPDIRECTINPUT8 directInput = nullptr;
         dynarray<LPDIRECTINPUTDEVICE8> joysticks;

         DIJOYSTATE curr_state;

      public:

         /*static int minInputRange;
         static int maxInputRange;*/

         Joystick() : directInput(NULL), joysticks(NULL)
         {
         }

         /*Joystick(int min, int max) : directInput(NULL), joysticks(NULL)
         {
            Joystick::minInputRange = min;
            Joystick::maxInputRange = max;
         }*/

         ~Joystick()
         {
            //delete directInput;
            //delete joystick;
         }

         //Chuck: Callback function of EnumDevices; called everytime direct input found a device (filtered),
         //diInstance is a pointer from directinput having info about the current device found
         static BOOL CALLBACK EnumJoystickCallback(const DIDEVICEINSTANCE* diInstance, VOID* pContext){

            auto context = reinterpret_cast<Joystick*>(pContext);

            LPDIRECTINPUTDEVICE8 currInput;

            HRESULT hr = context->directInput->CreateDevice(diInstance->guidInstance, &currInput, nullptr);
            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on interfacing with the controller");
               }
               assert(0);
            }

            context->joysticks.push_back(currInput);

            return DIENUM_CONTINUE;

         }

         static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* didoi, VOID* pContext){

            auto curr_joystick = reinterpret_cast<LPDIRECTINPUTDEVICE8>(pContext);

            if (didoi->dwType & DIDFT_AXIS){

               DIPROPRANGE diprg;
               diprg.diph.dwSize = sizeof(DIPROPRANGE);
               diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
               diprg.diph.dwHow = DIPH_BYID;
               diprg.diph.dwObj = didoi->dwType; // Specify the enumerated axis
               diprg.lMin = -120;
               diprg.lMax = 120;

               // Set the range for the axis
               if (FAILED(curr_joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
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
            if (joysticks.size() == 0)
            {
               return;
            }

            unsigned i = 0;
            for each (auto var in joysticks)
            {
               hr = var->SetDataFormat(&c_dfDIJoystick);
               if (FAILED(hr)){
                  if (DEBUG_EN){
                     printf("Failed to assign data format\n");
                  }
                  assert(0);
               }

               if (FAILED(hr = var->SetCooperativeLevel(window, DISCL_EXCLUSIVE | DISCL_BACKGROUND)))
                  assert(0);

               LPDIRECTINPUTDEVICE8 curr_joy = joysticks[i];
               //Chuck: enumerate the joystick objects. In callback we set range for the values.
               hr = var->EnumObjects(Joystick::EnumObjectsCallback, curr_joy, DIDFT_AXIS);

               if (FAILED(hr)){
                  if (DEBUG_EN){
                     printf("Failed on enumerating joystick objects\n");
                  }
                  assert(0);
               }

               i++;

            }
            //hr = joystick->SetDataFormat(&c_dfDIJoystick); //Chuck: specify what kind of structure we will have when using ::GetDeviceState
         }

         void AcquireInputData(int playerId){
            
            HRESULT hr;
            DIJOYSTATE js;

            if (joysticks[playerId] == nullptr)
               return; //js;

            //Chuck: Poll the device to read the current state
            hr = joysticks[playerId]->Poll();
            if (FAILED(hr))
            {
               //Chuck: input is interrupted, we aren't tracking any state between polls, so
               // we don't have any special reset that needs to be done. We just re-acquire and try again.
               hr = joysticks[playerId]->Acquire();
               return;// js;
            }

            //Chuck: get the input's device state
            hr = joysticks[playerId]->GetDeviceState(sizeof(DIJOYSTATE), &js);

            if (FAILED(hr)){
               if (DEBUG_EN){
                  printf("Failed on acquiring device data \n");
               }
               return;// js;
            }

            curr_state = js;

            return;// js;
         
         }
         
         DIJOYSTATE GetCurrentState(){
            return curr_state;
         }

         int GetNumberOfDevicesFound(){
            return joysticks.size();
         }
         
         void ShutDown()
         {
            if (joysticks.size() != 0){
               for each (auto curr_joy in joysticks)
               {
                  curr_joy->Unacquire();
                  curr_joy->Release();
               }
            }

            if (!directInput){
               directInput->Release();
            }
         }

   };
}


