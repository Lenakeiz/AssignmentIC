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
         
         if (rand->get(0.0f, 1.0f) >= 0.3f) players[currPlayerIdx]->counter++;
         //Chuck: Random behavior
         if (players[currPlayerIdx]->counter >= 30){
            players[currPlayerIdx]->MoveToHomePosition(rand->get(70,121));
            if (players[currPlayerIdx]->counter >= 35){
               players[currPlayerIdx]->counter = 0;
            }
            return;
         }
         
         btVector3 toTarget;
         btVector3 toCenter;
         float defBeh = 0.5f; //Chuck: BEH is behavior :D !! 
         float attBeh = 0.5f;

         //Chuck: here we decide the current player behavior 
         btVector3 COM = players[currPlayerIdx]->GetRigidBody()->getCenterOfMassPosition();
         btScalar distanceOnBoard = btVector3(COM.x(), 0, COM.z()).distance(btVector3(0,0,0));
         
         if (distanceOnBoard <= radius * 0.5f){
            curr_behav = Behavior::Aggressive;
         }
         else if (distanceOnBoard > radius * 0.5f && distanceOnBoard <= radius * 0.75f){
            //roll a dice for the balanced behavior (20 - 60% going to the center)
            attBeh = rand->get(0.0f, 1.0f); //0.7,0.9
            defBeh = 1 - attBeh;
            curr_behav = Behavior::Balanced;
         }
         else if (distanceOnBoard > radius * 0.75f && distanceOnBoard <= radius){
            curr_behav = Behavior::Defensive;
         }
         
         btVector3 currentNormalizeVelocityDirection;
         btVector3 linerInterpolation;
         switch (curr_behav)
         {
            case Behavior::Aggressive:
               if (FindTarget(players, currPlayerIdx, toTarget, currentNormalizeVelocityDirection)){
                  
                  btScalar dotProduct = currentNormalizeVelocityDirection.dot(toTarget);
                  //Dot product is projection (and remember vectors are normalized)
                  if (dotProduct >= 0.95f && players[currPlayerIdx]->GetPowerUpState(PowerUp::Dash) == PowerUpState::Activable){
                     players[currPlayerIdx]->ApplyDash();
                  }
                  else{
                     players[currPlayerIdx]->GetRigidBody()->applyCentralForce(toTarget * rand->get(90,131));
                  }
               }
               break;
            case Behavior::Defensive:
               RevertMotionTowardsCenter(players[currPlayerIdx], toCenter);
               players[currPlayerIdx]->GetRigidBody()->applyCentralForce(toCenter * 120);
               linerInterpolation = players[currPlayerIdx]->GetRigidBody()->getInterpolationLinearVelocity();
               if (linerInterpolation.norm() > 70.0f && players[currPlayerIdx]->GetPowerUpState(PowerUp::Massive) == PowerUpState::Activable){
                  players[currPlayerIdx]->ApplyMassive();
               }
               break;
            case Behavior::Balanced:
               RevertMotionTowardsCenter(players[currPlayerIdx], toCenter);
               if (FindTarget(players, currPlayerIdx, toTarget, currentNormalizeVelocityDirection)){
                  int forceStrength = rand->get(90,131);
                  btVector3 result = (toTarget * forceStrength * attBeh) + (toCenter * forceStrength * defBeh);
                  players[currPlayerIdx]->GetRigidBody()->applyCentralForce(result);
               }
               break;
            default:
               break;
         }

         if (availablePlayer.size() != 0) availablePlayer.reset();

      }

      bool FindTarget(dynarray<Player*> players, const int& currPlayerIdx, btVector3& pointing_vectorxz, btVector3& currentLinearVelNormalized){
         
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

            btVector3 linearVelNorm = players[currPlayerIdx]->GetRigidBody()->getLinearVelocity();
            if (linearVelNorm.norm() != 0.0f){
               currentLinearVelNormalized = linearVelNorm.normalize();
            }

            return true;

         }
         else return false;
      }
      
      void FindAndMoveToTarget(){}

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

