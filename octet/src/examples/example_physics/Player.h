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
      int lifes;
      float mass;
      ref<material> mat;
      ref<scene_node> node;
      ref<mesh_instance> meshinstance;
      btRigidBody* rigidBody; //Try implementing an non invasive (smart) pointer -- effective c++ 
   public:
   
      Player(btScalar diameter, btScalar height, Color color, int n = 4)
      {
         lifes = n;
         mass = 0.5f;

         switch (color) {
         case Color::RED : mat = new material(vec4(1, 0, 0, 1)); break;
         case Color::GREEN: mat = new material(vec4(0, 1, 0, 1)); break;
         case Color::BLUE: mat = new material(vec4(1, 0, 1, 1)); break;
         case Color::YELLOW: mat = new material(vec4(1, 1, 0, 1)); break;
         }

         //Creating default rigidbody
         mat4t modelToWorld;
         modelToWorld.loadIdentity();
         btCollisionShape *shape = new btCylinderShape(btVector3(diameter, height * 0.5f, diameter));
         btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
         btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
         btTransform transform(matrix, pos);
         btDefaultMotionState *motion = new btDefaultMotionState(transform);

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
      }

      void ApplyCentralForce(const btVector3& centralForce){
         rigidBody->applyCentralForce(centralForce);
      }

      void ApplyCentralImpulse(const btVector3& centralImpulse){
         rigidBody->applyCentralImpulse(centralImpulse);
      }

      ~Player()
      {
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

