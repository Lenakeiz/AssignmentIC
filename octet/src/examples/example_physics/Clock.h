#pragma once
#include <time.h>

namespace octet {

   class Clock
   {
      private:
         
         unsigned curr_ms;
         unsigned curr_sec;
         //unsigned curr_min; //Chuck: I will use this only if I will display the countdown as mm:ss
         unsigned target_seconds;
         int time_left;
         clock_t start_timer;      
      
      public:

         
         Clock() : start_timer(0), curr_ms(0), curr_sec(0), target_seconds(0), time_left(0){
            
         }

         Clock(unsigned sec) : start_timer(0), curr_ms(0), curr_sec(0), time_left(0)
         {
            target_seconds = sec;
         }

         ~Clock()
         {

         }

         void AssignTargetSec(int sec){
            target_seconds = sec;
         }

         void Reset(){
            start_timer = clock();
         }

         int GetTimeLeft(){
            return time_left;
         }

         bool TimeElapsed(){
            clock_t tick = clock();
            curr_ms = tick - start_timer;
            curr_sec = ((curr_ms / CLOCKS_PER_SEC)); //- curr_min * 60;
            //curr_min = (curr_ms / (CLOCKS_PER_SEC))/60;
            time_left = target_seconds - curr_sec;

            if (time_left < 0){
               time_left = 0;
               return true;
            }
            else
               return false;
         }


   };
}


