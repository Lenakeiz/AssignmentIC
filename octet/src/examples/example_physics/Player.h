#pragma once

namespace octet {
   enum class Color {
      RED,
      BLUE,
      GREEN,
      YELLOW
   };

   class Player
   {
   private:
      int lifes;
      float mass;
      ref<material> mat;
      ref<scene_node> node;
      ref<mesh_instance> meshinstance;
      btDefaultMotionState *motion; //Chuck: KEEP ATTENTION HOW RELEASE THIS
      btRigidBody* rigidBody; //Try implementing an non invasive (smart) pointer -- effective c++ 
   public:
   
      Player(btScalar diameter, btScalar height, Color color, btScalar arenaExtension, int n = 4)
      {
         lifes = n;
         mass = 0.5f;

         mat4t modelToWorld;
         
         switch (color) {
            case Color::RED: 
               mat = new material(vec4(1, 0, 0, 1));
               modelToWorld.translate(-(arenaExtension * 0.5f), 0, 0);
               break;
            case Color::GREEN: 
               mat = new material(vec4(0, 1, 0, 1));
               modelToWorld.translate(arenaExtension * 0.5f, 0, 0);
               break;
            case Color::BLUE:
               mat = new material(vec4(0, 0, 1, 1));
               modelToWorld.translate(0, 0, arenaExtension * 0.5f);
               break;
            case Color::YELLOW:
               mat = new material(vec4(1, 1, 0, 1));
               modelToWorld.translate(0, 0, -(arenaExtension * 0.5f));
               break;
         }

         //Creating default rigidbody
         btCollisionShape *shape = new btCylinderShape(btVector3(diameter, height * 0.5f, diameter));
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

         //Creating node to draw with mesh (cylinder mesh is created along z axis: scale and rotate)
         mat4t position;
         position.loadIdentity();
         position.scale(diameter, height * 0.5f, diameter);
         position.rotate(90, 1, 0, 0);
         mesh_cylinder* meshcylinder = new mesh_cylinder(zcylinder(), position, 50);
         node = new scene_node(modelToWorld, atom_);
         meshinstance = new mesh_instance(node, meshcylinder, mat);

         //delete shape; //for study
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

      btTransform GetTransform() {

         btTransform transform;
         if (motion) {
            motion->getWorldTransform(transform);
         }

         return transform;
      }

      void SetNodeToWorld(const mat4t& nodeToWorld){
         node->access_nodeToParent() = nodeToWorld;
      }

      void SetObjectToTheWorld(){
         btTransform playerTransform = this->GetTransform();
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

