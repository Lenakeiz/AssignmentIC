#pragma once
#include <time.h>

namespace octet {

   class Clock
   {
      private:
         
         unsigned curr_ms;
         unsigned curr_sec;
         unsigned curr_min;
         unsigned target_seconds;
         clock_t start_timer;      
      
      public:

         
         Clock() : start_timer(0), curr_ms(0), curr_min(0), curr_sec(0), target_seconds(0){
            
         }

         Clock(unsigned sec) : start_timer(0), curr_ms(0), curr_min(0), curr_sec(0)
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

         bool TimeElapsed(){
            clock_t tick = clock();
            curr_ms = tick - start_timer;
            curr_sec = ((curr_ms / CLOCKS_PER_SEC)); //- curr_min * 60;
            curr_min = (curr_ms / (CLOCKS_PER_SEC))/60;
            int time_left = target_seconds - curr_sec;
            return (time_left < 0);
         }


   };
}


