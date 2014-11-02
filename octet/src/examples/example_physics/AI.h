#pragma once

namespace octet{

   class AI : public resource{

   private:

      dynarray<Player*> availablePlayer;
      random* rand;

   public:

      ///Find a suitable player to fight;
      void MovePlayer(dynarray<Player*> players, int currPlayerIdx){
         
         //Chuck: Find the players with most lifes, if zero attack random
         int maxLife = 0;
        
         //Chuck: first screening
         for (unsigned i = 0; i < players.size(); i++)
         {
            int currLifes = players[i]->GetLifes();
            
            if (i != currPlayerIdx && currLifes > maxLife && players[i]->GetState() == PlayerState::Ingame){
               maxLife = currLifes;
               availablePlayer.push_back(players[i]);
            }
         }

         //second screening
         for (unsigned i = 0; i < availablePlayer.size(); i++){
            if (!(availablePlayer[i]->GetLifes() == maxLife)){
               availablePlayer.erase(i);
            }
         }

         int targetPlayer;
         if (availablePlayer.size() != 0)
         {
            targetPlayer = (int)rand->get(0, availablePlayer.size() - 1);

            btVector3 a = players[currPlayerIdx]->GetRigidBody()->getCenterOfMassPosition();
            btVector3 b = players[targetPlayer]->GetRigidBody()->getCenterOfMassPosition();
            btVector3 pointing_vector = b - a;

            btVector3 pointing_vectorxz(pointing_vector.x(), 0, pointing_vector.z());

            pointing_vectorxz = pointing_vectorxz.normalize();

            players[currPlayerIdx]->GetRigidBody()->applyCentralForce(pointing_vectorxz * 120);
         }
         
         availablePlayer.reset();

      }

      AI()
      {
         rand = new random();
      }

      ~AI()
      {
         delete rand;
      }

   };

}

