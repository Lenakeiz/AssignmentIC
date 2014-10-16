#pragma once

namespace octet{
   class Board
   {

   private:
      int current_tilt;
      ref<material> mat;
      ref<scene_node> node;
      ref<mesh_instance> meshinstance;
      btRigidBody* rigidBody; //Try implementing an non invasive (smart) pointer -- effective c++ 

   public:

   
      Board(btScalar diameter, btScalar height)
      {
         current_tilt = 0;
         //Assigning material
         mat = new material(new image("assets/Stone_floor_09.jpg"));

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
         shape->calculateLocalInertia(0.0f, inertia);
         //Saving rigid body 
         rigidBody = new btRigidBody(0.0f, motion, shape, inertia); //need to add this to the world (bullet physics) and also to the rigid bodies collection

         //Creating node to draw with mesh
         mat4t position;
         position.loadIdentity();
         position.scale(diameter, height * 0.5f, diameter);
         position.rotate(90, 1, 0, 0);
         mesh_cylinder* meshcylinder = new mesh_cylinder(zcylinder(), position, 50);
         node = new scene_node(modelToWorld, atom_);
         meshinstance = new mesh_instance(node, meshcylinder, mat);
         
      }

      ~Board()
      {
         delete rigidBody;
      }

      void Draw(){
      
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

