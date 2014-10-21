////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#pragma once
#include "Player.h"
#include "Board.h"

namespace octet {
  /// Scene using bullet for physics effects.
   static const char keyboardset[16] = { 'W', 'A', 'S', 'D',
      key_up, key_left, key_down, key_right,
      'T', 'F', 'G', 'H',
      'I', 'J', 'K', 'L' };

   class example_physics : public app {
      
      const int num_players = 4;
      enum {MaxPLayers = 4};
      //static const char keyboardset[MaxPLayers * 4];
      
      // scene for drawing box
      ref<visual_scene> app_scene;

      btDefaultCollisionConfiguration config;       /// setup for the world
      btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
      btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
      btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
      btDiscreteDynamicsWorld *world;               /// physics world, contains rigid bodies

      //dynarray<btRigidBody*> rigid_bodies;
      //dynarray<scene_node*> nodes;
      //dynarray<btUniversalConstraint*> constraints;
      
      //Chuck: adding array for players currently playing and board
      dynarray<Player*> players;
      Board *board;
      //Chuck: adding reference to the camera
      scene_node *scenecameranode;

      void add_sphere(mat4t_in modelToWorld, btScalar size, material *mat, bool is_dynamic = true) {

         btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
         btVector3 pos(get_btVector3(modelToWorld[3].xyz()));

         btCollisionShape *shape = new btSphereShape(size);

         btTransform transform(matrix, pos);

         btDefaultMotionState *motion = new btDefaultMotionState(transform);

         btScalar mass = is_dynamic ? 5.0f : 0.0f;
         btVector3 inertia;
         shape->calculateLocalInertia(mass, inertia);

         btRigidBody *rigidBody = new btRigidBody(mass, motion, shape, inertia);
         world->addRigidBody(rigidBody);
         //rigid_bodies.push_back(rigidBody);

         mesh_sphere *meshsphere = new mesh_sphere(vec3(0, 0, 0), size);
         scene_node *node = new scene_node(modelToWorld, atom_);
         //nodes.push_back(node);

         app_scene->add_child(node);
         app_scene->add_mesh_instance(new mesh_instance(node, meshsphere, mat));

      }

      void InitPhysics(){
         dispatcher = new btCollisionDispatcher(&config);
         broadphase = new btDbvtBroadphase();
         solver = new btSequentialImpulseConstraintSolver();
         world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
      }

      void ResetPhysics(){

         int i;
         for (i = world->getNumConstraints()-1; i >= 0; i--)
         {
            btTypedConstraint* constr = world->getConstraint(i);
            world->removeConstraint(constr);
            delete constr;
         }
         
         for ( i = world->getNumCollisionObjects()-1; i >= 0; i--)
         {
            btCollisionObject* obj = world->getCollisionObjectArray()[i];
            btRigidBody* rigidBody = btRigidBody::upcast(obj);
            if (rigidBody && rigidBody->getMotionState()){
               delete rigidBody->getMotionState();
            }
            world->removeCollisionObject(obj);
            delete obj;
         }

         players.reset();

         delete world;
         delete solver;
         delete broadphase;
         delete dispatcher;
      }

      void acquireInputs(){

         if (is_key_down(key_space))
         {
            ResetPhysics();
            InitPhysics();
            app_init();
         }
         
         for (unsigned i = 0; i < players.size(); i++)
         {
            btVector3 physics_vector(0, 0, 0);
            if (is_key_down(keyboardset[4 * i])){
               physics_vector += (btVector3(0, 0, -30));
            }
            if (is_key_down(keyboardset[4 * i + 1])){
               physics_vector += (btVector3(-30, 0, 0));
            }
            if (is_key_down(keyboardset[4 * i + 2])){
               physics_vector += (btVector3(0, 0, 30));
            }
            if (is_key_down(keyboardset[4 * i + 3])){
               physics_vector += (btVector3(30, 0, 0));
            }
            players[i]->ApplyCentralForce(physics_vector);
         }        

         //
         //players[1]->ApplyCentralForce(physics_vector);
         //players[2]->ApplyCentralForce(physics_vector);
         //players[3]->ApplyCentralForce(physics_vector);

         //physics_vector = btVector3(0,0,0);

         //if (is_key_down(key_up)){
         //   physics_vector += btVector3(0, 0, -30);
         //   //players[1]->ApplyCentralForce(btVector3(0, 0, -60));
         //}
         //if (is_key_down(key_left)){
         //   physics_vector += btVector3(-30, 0, 0);
         //   //players[1]->ApplyCentralForce(btVector3(-60, 0, 0));
         //}
         //if (is_key_down(key_down)){
         //   physics_vector += btVector3(0, 0, 30);
         //   //players[1]->ApplyCentralForce(btVector3(0, 0, 60));
         //}
         //if (is_key_down(key_right)){
         //   physics_vector += btVector3(30, 0, 0);
         //   //players[1]->ApplyCentralForce(btVector3(60, 0, 0));
         //}

         //players[1]->ApplyCentralForce(physics_vector);

      }
   
      public:
         /// this is called when we construct the class before everything is initialised.
         example_physics(int argc, char **argv) : app(argc, argv) {
         
            InitPhysics();
         
         /*keyboardset = { 'W', 'A', 'S', 'D',
            key_up, key_left, key_down, key_right,
            'T', 'F', 'G', 'H',
            'I', 'J', 'K', 'L' };*/

         }

         ~example_physics() {
         
            ResetPhysics();

         }

         /// this is called once OpenGL is initialized
         void app_init() {

            app_scene =  new visual_scene();

            app_scene->create_default_camera_and_lights();
            scenecameranode = app_scene->get_camera_instance(0)->get_node();

            //Chuck: camera is fixed, changing the parameters in order to have camera looking at the platform along Z-axis
            scenecameranode->access_nodeToParent().rotateX(-30);
            scenecameranode->access_nodeToParent().translate(0,20,20);
            world->setGravity(btVector3(0,-40,0)); //To prevent strange behaviour on the collisions (on over the other)
            // add the ground (as a static object)
            btScalar boardRadius = 20.0f;
            btScalar boardhalfheight = 0.5f;
            mat4t modelToWorld;
            modelToWorld.loadIdentity();

            board = new Board(boardRadius, boardhalfheight);
            world->addRigidBody(board->GetRigidBody());
            //rigid_bodies.push_back(board->GetRigidBody());
            //nodes.push_back(board->GetNode());
            app_scene->add_child(board->GetNode());
            app_scene->add_mesh_instance(board->GetMesh());

            for (int i = 0; i < num_players; i++)
            {
               material* mat;
               modelToWorld.loadIdentity();

               switch ((Color)i)
               {
               case Color::RED:
                  mat = new material(vec4(1, 0, 0, 1));
                  modelToWorld.translate(-(boardRadius * 0.5f), 10.0f, -0);
                  break;
               case Color::GREEN:
                  mat = new material(vec4(0, 1, 0, 1));
                  modelToWorld.translate(boardRadius * 0.5f, 10.0f, 0);
                  break;
               case Color::BLUE:
                  mat = new material(vec4(0, 0, 1, 1));
                  modelToWorld.translate(0, 10.0f, boardRadius * 0.5f);
                  break;
               case Color::YELLOW:
                  mat = new material(vec4(1, 1, 0, 1));
                  modelToWorld.translate(0, 10.0f, -(boardRadius * 0.5f));
                  break;
                  default:
                     break;
               }

               Player *player = new Player(2.0f, 0.5f, *mat, modelToWorld); //Chuck: assign a transform here to pass to player, the player does not need to know board dimension
               
               world->addRigidBody(player->GetRigidBody());
               
               btTransform localConstr;
               localConstr.setIdentity();
               btVector3 playerCOM = player->GetRigidBody()->getCenterOfMassPosition();               
               //localConstr.setOrigin(btVector3(-(playerCOM.getX()), -(playerCOM.getY()), -(playerCOM.getZ())));
               btGeneric6DofConstraint* constr = new btGeneric6DofConstraint((*player->GetRigidBody()), localConstr, true);//(*player->GetRigidBody()),*board->GetRigidBody(), localConstr, localConstr2, false
                              
               world->addConstraint(constr);
               constr->setLinearLowerLimit(btVector3(-boardRadius - (playerCOM.getX()) - 20, -100 - playerCOM.getY(), -boardRadius - (playerCOM.getZ()) - 20 ));
               constr->setLinearUpperLimit(btVector3(boardRadius - (playerCOM.getX()) + 20, 100, boardRadius - (playerCOM.getZ()) + 20));
               constr->setAngularLowerLimit(btVector3(0, 0, 0));
               constr->setAngularUpperLimit(btVector3(0, 0, 0));

               //rigid_bodies.push_back(player->GetRigidBody());
               //nodes.push_back(player->GetNode());
               app_scene->add_child(player->GetNode());
               app_scene->add_mesh_instance(player->GetMesh());
               players.push_back(player);
            }

         }

         /// this is called to draw the world
         void draw_world(int x, int y, int w, int h) {
            
            world->stepSimulation(1.0f/30);            

            for (unsigned i = 0; i != players.size(); i++)
            {
               players[i]->SetObjectToTheWorld();
            }

            /*for (unsigned i = 0; i != rigid_bodies.size(); ++i) {
               btRigidBody *rigid_body = rigid_bodies[i];
               btQuaternion btq = rigid_body->getOrientation();
               btVector3 pos = rigid_body->getCenterOfMassPosition();
               quat q(btq[0], btq[1], btq[2], btq[3]);
               mat4t modelToWorld = q;
               modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);
               nodes[i]->access_nodeToParent() = modelToWorld;
            }*/

            acquireInputs();
            // update matrices. assume 30 fps.
            app_scene->update(1.0f/30);

            int vx = 0, vy = 0;

            get_viewport_size(vx, vy);
            app_scene->begin_render(vx, vy);

            
            // draw the scene
            app_scene->render((float)vx / vy);
         }
      };
}
