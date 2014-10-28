#pragma once
#include "Clock.h"

namespace octet {
   
   enum { NUM_POWERUPS = 4 };

   enum class Color { RED, BLUE, GREEN, YELLOW };

   class Player
   {
   private:
      int lifes;
      Color color;
      float mass;
      bool active;

      //Timers for powerups and cooldowns
      bool powerups[NUM_POWERUPS];
      Clock timers[NUM_POWERUPS];
      
      ref<material> mat;
      ref<scene_node> node;
      ref<mesh_instance> meshinstance;
      btDefaultMotionState *motion; //Chuck: KEEP ATTENTION HOW RELEASE THIS
      btRigidBody* rigidBody; //Try implementing an non invasive (smart) pointer -- effective c++
      btGeneric6DofConstraint* constraint;      

   public:
   
      Player(btScalar radius, btScalar halfheight, material& material, Color playerColor, const mat4t& modelToWorld, int n = 4)
      {
         
         ResetPowerUps();
         
         lifes = n;
         active = true;
         mass = 0.5f;

         this->mat = &material;
         this->color = playerColor;
         //Creating default rigidbody
         btCollisionShape *shape = new btCylinderShape(btVector3(radius, halfheight, radius));
         btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
         btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
         btTransform transform(matrix, pos);
         motion = new btDefaultMotionState(transform);

         //Calculate inertia for the body
         btVector3 inertia;
         shape->calculateLocalInertia(mass, inertia);
         //Saving rigid body 
         rigidBody = new btRigidBody(mass, motion, shape, inertia); //need to add this to the world (bullet physics) and also to the rigid bodies collection

         //prevent body from deactivating
         rigidBody->setActivationState(DISABLE_DEACTIVATION);
         rigidBody->setRestitution(1);
         //Creating node to draw with mesh (cylinder mesh is created along z axis: scale and rotate)
         mat4t position;
         position.loadIdentity();
         position.scale(radius, halfheight, radius);
         position.rotate(90, 1, 0, 0);
         mesh_cylinder* meshcylinder = new mesh_cylinder(zcylinder(), position, 50);
         node = new scene_node(modelToWorld, atom_);
         meshinstance = new mesh_instance(node, meshcylinder, mat);

      }

      void ResetPowerUps(){

         for (unsigned i = 0; i < NUM_POWERUPS - 1; i++)
         {
            powerups[i] = false;
         }

         timers[0].AssignTargetSec(3);
         timers[1].AssignTargetSec(3);
         timers[2].AssignTargetSec(30); //DASH
         timers[3].AssignTargetSec(3);

      }

      void ApplyPowerUps(BYTE rgbButtons[], int size){

         for (unsigned i = 0; i < size; i++){
            if (!powerups[i] && (rgbButtons[i] & 0x80)){
               
               //if pressed apply powerups
               btVector3 linearVelNorm;
               switch (i){
                  case 0:
                     //Chuck: not implemented
                     break;
                  case 1:
                     //Chuck: not implemented
                     break;
                  case 2:
                     //Chuck: DASH
                     linearVelNorm = rigidBody->getLinearVelocity();
                     linearVelNorm = linearVelNorm.normalize();
                     rigidBody->applyCentralImpulse(linearVelNorm * 20);
                     break;
                  case 3:
                     //Chuck: not implemented
                     break;
                  default:
                     break;
               }

               powerups[i] = true;
               timers[i].Reset();
            }
         }
      }

      void CheckPowerUps(){
         for (unsigned i = 0; i < NUM_POWERUPS; i++)
         {
            if (powerups[i]){
               if (timers[i].TimeElapsed()){
                  powerups[i] = false;
               }
            }
         }
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

      void SetActive(bool enabled){
         active=enabled;
      }

      bool GetActive(){
         return active;
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

