#pragma once

namespace octet{

   class AI : public resource{

   private:      
      dynarray<int> avaiblePlayer;

   public:

      void MovePlayer(dynarray<Player*> players, int currPlayerIdx){
         
         //Find the players with most lifes, if zero attack random
         int maxLife = 1;
         int lastIndex = 0;

         for (unsigned i = 0; i < players.size(); i++)
         {
            int currLifes = players.size[i]->GetLifes();
            
            if (i != currPlayerIdx && currLifes >= maxLife && players[i]->GetState() == PlayerState::Ingame){
               maxLife = currLifes;
               avaiblePlayer.push_back(i);
            }
         }

         avaiblePlayer.reset();

      }

      AI()
      {

      }

      ~AI()
      {

      }

   };

}

