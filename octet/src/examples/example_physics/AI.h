#pragma once

namespace octet{

   enum class Behavior { Aggressive = 1, Defensive = 2, Balanced = 3};

   class AI : public resource{

   private:

      dynarray<Player*> availablePlayer;
      Behavior curr_behav;
      octet::math::random* rand;
      
   public:

      ///Find a suitable player to fight;
      void MovePlayer(dynarray<Player*> players, const int currPlayerIdx, const btScalar radius){
         
         btVector3 toTarget;
         btVector3 toCenter;
         float defBeh = 0.5f; //Chuck: BEH is behavior :D !! 
         float attBeh = 0.5f;

         //Chuck: here we decide the current player behavior 
         btVector3 COM = players[currPlayerIdx]->GetRigidBody()->getCenterOfMassPosition();
         btScalar distance = btVector3(COM.x(), 0, COM.z()).distance(btVector3(0,0,0));
         
         if (distance <= radius * 0.5f){
            curr_behav = Behavior::Aggressive;
            if (DEBUG_EN)
               printf("Aggressive");
         }
         else if (distance > radius * 0.5f && distance <= radius * 0.75f){
            //roll a dice for the balanced behavior (20 - 60% going to the center)
            attBeh = rand->get(0.2f, 0.8f);
            defBeh = 1 - attBeh;
            curr_behav = Behavior::Balanced;
            if (DEBUG_EN)
               printf("Balanced");
         }
         else if (distance > radius * 0.75f && distance <= radius){
            curr_behav = Behavior::Defensive;
            if (DEBUG_EN)
               printf("Defensive");
         }
         
         switch (curr_behav)
         {
            case Behavior::Aggressive:
               if (FindTarget(players, currPlayerIdx, toTarget)){
                  players[currPlayerIdx]->GetRigidBody()->applyCentralForce(toTarget * 120);
               }
               break;
            case Behavior::Defensive:
               RevertMotionTowardsCenter(players[currPlayerIdx], toCenter);
               players[currPlayerIdx]->GetRigidBody()->applyCentralForce(toCenter * 120);
               break;
            case Behavior::Balanced:
               RevertMotionTowardsCenter(players[currPlayerIdx], toCenter);
               if (FindTarget(players, currPlayerIdx, toTarget)){
                  btVector3 result = (toTarget * 120 * attBeh) + (toCenter * 120 * defBeh);
                  players[currPlayerIdx]->GetRigidBody()->applyCentralForce(result);
               }
               break;
            default:
               break;
         }

         if (availablePlayer.size() != 0) availablePlayer.reset();

      }

      bool FindTarget(dynarray<Player*> players, const int& currPlayerIdx, btVector3& pointing_vectorxz){
         
         //Chuck: Find the players with most lifes, if more than one attack random
         int maxLife;

         getMaxLife(players, currPlayerIdx, maxLife);

         //second screening
         for (unsigned i = 0; i < availablePlayer.size(); i++){
            if (!(availablePlayer[i]->GetLifes() == maxLife)){
               availablePlayer.erase(i);
            }
         }

         int targetPlayer;
         if (availablePlayer.size() != 0)
         {
            int targetPlayer = rand->get(0, (availablePlayer.size())); //Chuck: don' t need to add -1 since the round it's always by defect

            pointing_vectorxz = getNormalizedDistanceVector(availablePlayer[targetPlayer]->GetRigidBody()->getCenterOfMassPosition(), players[currPlayerIdx]->GetRigidBody()->getCenterOfMassPosition());

            return true;
         }
         else return false;

         
      }
      
      void FindAndMoveToTarget(){
      }

      void RevertMotionTowardsCenter(Player* currPlayer, btVector3& pointing_vectorxz){
         
         pointing_vectorxz = getNormalizedDistanceVector(btVector3(0, 0, 0), currPlayer->GetRigidBody()->getCenterOfMassPosition());
         
      }
      
      
      const btVector3& getNormalizedDistanceVector(const btVector3& b, const btVector3& a){

         btVector3 pointing_vector = b - a;

         btVector3 pointing_vectorxz = btVector3(pointing_vector.x(), 0, pointing_vector.z());

         pointing_vectorxz = pointing_vectorxz.normalize();

         return pointing_vectorxz;
      }

      void getMaxLife(dynarray<Player*> players, const int& currPlayerIdx, int& life){
         
         life = 0;

         //Chuck: first screening
         for (unsigned i = 0; i < players.size(); i++)
         {
            int currLifes = players[i]->GetLifes();

            if (i != currPlayerIdx && currLifes >= life && players[i]->GetState() == PlayerState::Ingame){
               life = currLifes;
               availablePlayer.push_back(players[i]);
            }
         }
      }

      AI()
      {
         rand = new random(0x5656F);
      }

      ~AI()
      {
         delete rand;
      }

   };

}

