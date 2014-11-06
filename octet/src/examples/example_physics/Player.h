#pragma once
#include "Clock.h"

namespace octet {
   
   enum { NUM_POWERUPS = 4 };
   enum class PlayerState { Ingame, Inactive, KO, Dead, Respawing };
   enum class PowerUp { Undefinied0, Undefinied1, Dash, Massive };
   enum class PowerUpState { Activable, Active, Cooldown };
   enum class Color { RED, BLUE, GREEN, YELLOW };

   class Player
   {
   private:

      const unsigned activeTime = 2;
      const unsigned cooldownTime = 6;

      int lifes;
      Color color;
      PlayerState state;
      float initial_mass;
      float curr_mass;
      btScalar radius;
      btScalar halfheight;
      bool active;
      bool ingame;
      bool aicontrolled;

      //Timers for powerups and cooldowns
      dynarray<PowerUpState> powerups;
      dynarray<Clock*> timers;
      
      ref<material> mat;
      ref<material> metalmat;
      ref<scene_node> node;
      ref<mesh_instance> meshinstance;
      btDefaultMotionState *motion; //Chuck: KEEP ATTENTION HOW RELEASE THIS
      btRigidBody* rigidBody; //Try implementing an non invasive (smart) pointer -- effective c++
      btGeneric6DofConstraint* constraint;      

   public:
   
      Player(btScalar radius, btScalar halfheight, material& material, Color playerColor, const mat4t& modelToWorld, bool ai ,int n = 4)
      {
         
         ResetPowerUps();
         
         this->radius = radius;
         this->halfheight = halfheight;

         lifes = n;
         this->aicontrolled = ai;
         state = PlayerState::Ingame;
         curr_mass = initial_mass = 0.5f;

         this->mat = &material;
         metalmat = new octet::material(new image("assets/metal.gif"));
         this->color = playerColor;
         //Creating default rigidbody
         btCollisionShape *shape = new btCylinderShape(btVector3(radius, halfheight, radius));
         btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
         btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
         btTransform transform(matrix, pos);
         motion = new btDefaultMotionState(transform);

         //Calculate inertia for the body
         btVector3 inertia;
         shape->calculateLocalInertia(initial_mass, inertia);
         //Saving rigid body 
         rigidBody = new btRigidBody(initial_mass, motion, shape, inertia); //need to add this to the world (bullet physics) and also to the rigid bodies collection

         //prevent body from deactivating
         rigidBody->setActivationState(DISABLE_DEACTIVATION);
         rigidBody->setRestitution(1);
         //Creating node to draw with mesh (cylinder mesh is created along z axis: scale and rotate)
         mat4t position;
         position.loadIdentity();
         position.scale(radius, halfheight, radius);
         position.rotate(90, 1, 0, 0);
         ref<mesh_cylinder> meshcylinder = new mesh_cylinder(zcylinder(), position, 50);
         node = new scene_node(modelToWorld, atom_);
         meshinstance = new mesh_instance(node, meshcylinder, mat);

      }

      void ResetPowerUps(){

         for (unsigned i = 0; i < NUM_POWERUPS ; i++)
         {
            powerups.push_back(PowerUpState::Activable);
            timers.push_back(new Clock());
         }

      }

      //Chuck: reset player aspect after active state is expired
      void ResetPlayerAspect(PowerUp pw){

         btVector3 initialInertia;

         switch (pw)
         {
            case octet::PowerUp::Undefinied0:
               //Chuck: do nothing
               break;
            case octet::PowerUp::Undefinied1:
               //Chuck: do nothing
               break;
            case octet::PowerUp::Dash:
               //Chuck: do nothing
               break;
            case octet::PowerUp::Massive:
               curr_mass = initial_mass;
               rigidBody->getCollisionShape()->calculateLocalInertia(curr_mass,initialInertia);
               rigidBody->setMassProps(curr_mass,initialInertia);
               meshinstance->set_material(mat);
               break;
            default:
               break;
         }
      }

      void ApplyPowerUps(BYTE* rgbButtons){

         btVector3 linearVelNorm;
         btVector3 newInertia;
         for (unsigned i = 0; i < 4; i++){
            if ((rgbButtons[i] & 0x80) && powerups[i] == PowerUpState::Activable){
               
               //if pressed apply powerups
               PowerUpState nextState = powerups[i]; //setting to current state
               
               switch (i){
                  case 0:
                     //Chuck: not implemented
                     break;
                  case 1: 
                     //Chuck: not implemented
                     break;
                  case 2:
                     //Chuck: DASH: using linear velocity to get directin of movement, applying an impulse to that
                     linearVelNorm = rigidBody->getLinearVelocity();
                     linearVelNorm = linearVelNorm.normalize();
                     rigidBody->applyCentralImpulse(btVector3(linearVelNorm.x(),0,linearVelNorm.y())*40);
                     nextState = PowerUpState::Cooldown;
                     timers[i]->AssignTargetSec(cooldownTime);
                     break;
                  case 3:
                     //Chuck: MASSIVE
                     curr_mass = initial_mass * 20;
                     rigidBody->getCollisionShape()->calculateLocalInertia(curr_mass, newInertia);
                     rigidBody->setMassProps(curr_mass, newInertia);
                     rigidBody->setLinearVelocity(btVector3(0, 0, 0));
                     meshinstance->set_material(metalmat);
                     nextState = PowerUpState::Active;
                     timers[i]->AssignTargetSec(activeTime);
                     break;
                  default:
                     break;
               }

               powerups[i] = nextState;
               timers[i]->Reset();

            }
         }
      }

      void CheckPowerUps(){

         for (unsigned i = 0; i < NUM_POWERUPS; i++)
         {
            if (powerups[i]==PowerUpState::Active){
               if (timers[i]->TimeElapsed()){
                  powerups[i] = PowerUpState::Cooldown;
                  ResetPlayerAspect((PowerUp)i);
                  timers[i]->AssignTargetSec(cooldownTime);
                  timers[i]->Reset();
               }
            }
            else if (powerups[i] == PowerUpState::Cooldown){
               if (timers[i]->TimeElapsed())
               {
                  powerups[i] = PowerUpState::Activable;
               }
            }
         }

      }
      
      const btScalar GetHalfHeight(){
         return this->halfheight;
      }

      const btScalar GetRadius(){
         return this->radius;
      }

      const char* GetColorString(){

         switch (this->color){
         case Color::RED: return "Red"; break;
         case Color::GREEN: return "Green"; break;
         case Color::BLUE: return "Blue"; break;
         case Color::YELLOW: return "Yellow"; break;
         default: return ""; break;
         }

      }

      const Color GetColor(){
         return (this->color);
      }

      const char* GetPowerUpString(PowerUp pow){
         switch(pow)
         {
            case PowerUp::Undefinied0: return "Unavailable";
            case PowerUp::Undefinied1: return "Unavailable";
            case PowerUp::Dash: return "Dash";
            case PowerUp::Massive: return "Massive";
         }
      }

      bool GetAiEnabled(){
         return aicontrolled;
      }

      void SetState(PlayerState state){
         this->state=state;
      }

      PlayerState GetState(){
         return this->state;
      }

      void SetLife(unsigned lifes){
         this->lifes = lifes;
      }

      void DecreaseLife(){
         lifes--;
      }

      int GetLifes(){
         return lifes;
      }

      bool IsLifeEnd(){
         return lifes == 0;
      }

      void ApplyCentralForce(const btVector3& centralForce){
         rigidBody->applyCentralForce(centralForce);
      }

      void ApplyCentralImpulse(const btVector3& centralImpulse){
         rigidBody->applyCentralImpulse(centralImpulse);
      }

      void SetLinearVelocity(const btVector3& linearVelocity){
         rigidBody->setLinearVelocity(linearVelocity);
      }

      void AddConstraintInfo(btGeneric6DofConstraint* constr){
         constraint = constr;
      }

      void SetConstraintEnabled(bool enabled){
         constraint->setEnabled(enabled);
      }
         
      btTransform GetTransform() {

         btTransform transform;
         if (motion) {
            motion->getWorldTransform(transform);
         }

         return transform;
      }

      void SetTransform(const btTransform& tra){

         if (motion) {
            motion->setWorldTransform(tra);
         }

      }

      void SetNodeToWorld(const mat4t& nodeToWorld){
         node->access_nodeToParent() = nodeToWorld;
      }

      void SetObjectToTheWorld(){
         btTransform playerTransform = this->GetTransform();
         //btTransform playerTransform;
         //this->GetRigidBody()->getMotionState()->getWorldTransform(playerTransform);// GetTransform();
         btQuaternion btq = playerTransform.getRotation();
         btVector3 com = playerTransform.getOrigin();
         quat q(btq[0], btq[1], btq[2], btq[3]);
         mat4t modelToWorld = q;
         modelToWorld[3] = vec4(com[0], com[1], com[2], 1);
         this->SetNodeToWorld(modelToWorld);
      }

      const string GetInfoString(){
         string retString;
         retString.format("Player %s \nLife : %d \nMass : %f\n", GetColorString(), lifes, curr_mass);
         string pow;
         for (int i = 0; i < powerups.size(); i++)
         {
            pow.format("%s time left: %d \n", GetPowerUpString((PowerUp)i), timers[i]->GetTimeLeft());
            retString += pow;
         }

         if (DEBUG_EN){
            btVector3 position = rigidBody->getCenterOfMassPosition();
            string pos;
            pos.format("x: %f\ny: %f\nyz: %f\n", position.x(), position.y(), position.z());
            retString += pos;
         }

         retString += "\0";

         return retString;
      }

      ~Player()
      {
         delete motion;
         delete rigidBody;
      }

      mesh_instance* GetMesh(){
         return meshinstance;
      }

      btRigidBody* GetRigidBody(){
         return rigidBody;
      }

      scene_node* GetNode(){
         return node;
      }

   };
}

