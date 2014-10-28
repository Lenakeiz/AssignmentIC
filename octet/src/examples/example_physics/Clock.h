#pragma once
#include <time.h>

namespace octect {

   class Clock
   {
      private:
         
         unsigned curr_ms;
         int target_seconds;
         clock_t start_timer;      
      
      public:

         Clock(unsigned sec) : start_timer(0), curr_ms(0)
         {
            target_seconds = sec;
         }

         ~Clock()
         {

         }

         void Reset(){
            start_timer = clock();
         }

         void AssignTargetSec(int sec){
            target_seconds =  sec;
         }

         bool TimeElapsed(){
            clock_t tick = clock();
            curr_ms = tick - start_timer;
            int sec = (curr_ms/CLOCKS_PER_SEC)/60;
            return (target_seconds - sec < 0);
         }


   };
}


