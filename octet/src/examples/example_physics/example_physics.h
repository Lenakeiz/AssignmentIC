////////////////////////////////////////////////////////////////////////////////
//
// (C) Andrea Castegnaro 2014
//
//
//
#pragma once
#define DEBUG_EN 1
#include "Player.h"
#include "Board.h"
#include "Joystick.h"


namespace octet {
  /// Scene using bullet for physics effects.
   static const char keyboardset[16] = { 'W', 'A', 'S', 'D',
      key_up, key_left, key_down, key_right,
      'T', 'F', 'G', 'H',
      'I', 'J', 'K', 'L' };

   class example_physics : public app {
   
      Joystick* joystick = nullptr;

      int num_players;
      enum {MaxPLayers = 4};
      const int gravity = -120;
      //static const char keyboardset[MaxPLayers * 4];

      
      // scene for drawing box
      ref<visual_scene> app_scene;

      btDefaultCollisionConfiguration config;       /// setup for the world
      btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
      btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
      btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
      btDiscreteDynamicsWorld *world;               /// physics world, contains rigid bodies

      //Chuck: adding array for players currently playing and board
      ref<mesh_instance> background;
      dynarray<Player*> players;
      ref<Board> board;
      
      //Chuck: adding reference to the camera
      ref<camera_instance> scenecamera;

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

      void ResetPlayers(){

         btScalar boardRadius = board->GetRadius();
         btScalar boardhalfheight = board->GetHalfHeight();

         mat4t modelToWorld;        

         for each (auto var in players)
         {
            modelToWorld.loadIdentity();
            
            var->GetRigidBody()->clearForces();
            var->SetActive(false);

            switch (var->GetColor())
            {
            case Color::RED:
               modelToWorld.translate(-(boardRadius * 0.5f), boardhalfheight * 2, 0);
               break;
            case Color::GREEN:
               modelToWorld.translate(boardRadius * 0.5f, boardhalfheight * 2, 0);// var->GetRigidBody()->translate(btVector3(boardRadius * 0.5f, boardhalfheight * 2, 0));
               break;
            case Color::BLUE:
               modelToWorld.translate(0, boardhalfheight * 2, boardRadius * 0.5f);// var->GetRigidBody()->translate(btVector3(0, boardhalfheight * 2, boardRadius * 0.5f));
               break;
            case Color::YELLOW:
               modelToWorld.translate(0, boardhalfheight * 2, -(boardRadius * 0.5f));// var->GetRigidBody()->translate(btVector3(0, boardhalfheight * 2, -(boardRadius * 0.5f)));
               break;
            default:
               break;
            }

            btTransform trans(get_btMatrix3x3(modelToWorld), get_btVector3(modelToWorld[3].xyz()));
            var->SetTransform(trans); //translate(btVector3(-(boardRadius * 0.5f), boardhalfheight * 2, 0));

         }
      }

      void GameReset(){
         ResetPhysics();
         InitPhysics();
         joystick->ShutDown(); //Chuck: THIS SET OF INSTRUCTION IS A GAME RESET
         delete joystick;
         app_init();
      }

      void InitPhysics(){
         dispatcher = new btCollisionDispatcher(&config);
         broadphase = new btDbvtBroadphase();
         solver = new btSequentialImpulseConstraintSolver();
         world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
         world->setGravity(btVector3(0, gravity, 0)); //To prevent strange behaviour on the collisions (one over the other)
      }

      void ResetPhysics(){

         int i;
         for (i = world->getNumConstraints()-1; i >= 0; i--)
         {
            btTypedConstraint* constr = world->getConstraint(i);
            world->removeConstraint(constr);
            delete constr;
         }
         
         for ( i = world->getNumCollisionObjects() - 1; i >= 0; i--)
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

         if (is_key_down(key_esc)) exit(1);

         if (is_key_down(key_space))
         {
            GameReset();
         }
         else{
            for (unsigned i = 0; i < players.size(); i++)
            {
               btVector3 physics_vector(0, 0, 0);
               if (players[i]->GetActive()){
                  if (is_key_down(keyboardset[4 * i])){
                     physics_vector += (btVector3(0, 0, -120));
                  }
                  if (is_key_down(keyboardset[4 * i + 1])){
                     physics_vector += (btVector3(-120, 0, 0));
                  }
                  if (is_key_down(keyboardset[4 * i + 2])){
                     physics_vector += (btVector3(0, 0, 120));
                  }
                  if (is_key_down(keyboardset[4 * i + 3])){
                     physics_vector += (btVector3(120, 0, 0));
                  }
               }
               players[i]->ApplyCentralForce(physics_vector);
            }

            //just for player zero we take input from controller
            btVector3 joyInput = joystick->AcquireInputData();
            players[0]->ApplyCentralForce(joyInput);
         }
      }
   
      void checkPLayersStatus(){

         btVector3 boardtrans = board->GetRigidBody()->getCenterOfMassPosition();
         btVector3 boardtransXZ(boardtrans.getX(),0,boardtrans.getZ());
         btScalar boardY = boardtrans.getY();

         for (unsigned i = 0; i < players.size(); i++)
         {
            btRigidBody* currBody = players[i]->GetRigidBody();
            //btTransform trans = currBody->getCenterOfMassTransform();
            btVector3 currPLayer = currBody->getCenterOfMassPosition(); //trans.getOrigin();

            btVector3 currPLayerXZ(currPLayer.getX(), 0, currPLayer.getZ());
            btScalar playY = currPLayer.getY();
            
            btScalar distanceToBoard = currPLayerXZ.distance(boardtransXZ);
            btScalar boardRadius = board->GetRadius();

            btScalar lower_offset = 0.2;
            btScalar upper_offset = 0.5;

            if (players[i]->GetActive() == true){
               if (distanceToBoard >= boardRadius + upper_offset || math::abs(playY - boardY) > 2.0 + 1.0 + upper_offset + 0.5){
                  players[i]->SetActive(false);
                  if (DEBUG_EN){
                     printf("Player %s false \n", players[i]->GetColorString());
                  } 
               }
            }
            else{
               if ((math::abs(playY - boardY) <= 2.0 + 1.0 + lower_offset) && distanceToBoard <= board->GetRadius() + lower_offset){
                  players[i]->SetActive(true);
                  if (DEBUG_EN){
                     printf("Player %s true \n", players[i]->GetColorString());
                  }
               }
            }
         }
      }

      public:
         /// this is called when we construct the class before everything is initialised.
         example_physics(int argc, char **argv) : app(argc, argv) {         
            InitPhysics();
         }

         ~example_physics() {         
            ResetPhysics();
            joystick->ShutDown();
         }

         /// this is called once OpenGL is initialized
         void app_init() {

            num_players = 4;

            joystick = new Joystick();
            joystick->InitInputDevice(this);

            /*btScalar boardRadius = 40.0f;
            btScalar boardhalfheight = 2.0f;*/
            vec3 boardsize(40.0f,2.0f,40.0f);
            btScalar back_size = 300;

            app_scene = new visual_scene();

            app_scene->create_default_camera_and_lights();
            scenecamera = app_scene->get_camera_instance(0);

            //Chuck: camera is fixed, changing the parameters in order to have camera looking at the platform along Z-axis
            scenecamera->get_node()->access_nodeToParent().rotateX(-30);
            scenecamera->get_node()->access_nodeToParent().translate(0, 20, boardsize.x() * 2);
            auto p = scenecamera->get_far_plane();
            scenecamera->set_far_plane(500); //->set_perspective(0.1f, 45, 1, 0.00001f, 500); //why using set perspective does not work???
            
            mat4t modelToWorld;
            modelToWorld.loadIdentity();

            //Chuck: setting the background
            material *back_mat = new material(new image("assets/supernova.gif"));
            mesh_sphere *meshsphere = new mesh_sphere(vec3(0, 0, 0), back_size);
            scene_node *node = new scene_node(modelToWorld, atom_);
            node->rotate(180, vec3(0,1,0));
            node->rotate(10, vec3(1, 0, 0));
            app_scene->add_child(node);
            background = new mesh_instance(node, meshsphere, back_mat);
            app_scene->add_mesh_instance(background);
            
            //Chuck: add the ground (as a static object)
            modelToWorld.loadIdentity();
            board = new Board(boardsize);
            world->addRigidBody(board->GetRigidBody());

            app_scene->add_child(board->GetNode());
            app_scene->add_mesh_instance(board->GetMesh());
            
            //modelToWorld.loadIdentity();
            //btCollisionShape *shape = new btBoxShape(btVector3(boardRadius * 2, boardhalfheight * 0.5, boardRadius * 2));
            //btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
            //btVector3 pos(get_btVector3(modelToWorld[3].xyz()));
            //btTransform transform(matrix, pos);
            //btDefaultMotionState *motion = new btDefaultMotionState(transform);

            ////Calculate inertia for the body
            //btVector3 inertia;
            //shape->calculateLocalInertia(0.0f, inertia);
            ////Saving rigid body 
            //btRigidBody *deadBox = new btRigidBody(0.0f, motion, shape, inertia); //need to add this to the world (bullet physics) and also to the rigid bodies collection

            //world->addRigidBody(deadBox);
            
            for (int i = 0; i < num_players; i++)
            {
               material* mat;
               modelToWorld.loadIdentity();
               
               switch ((Color)i)
               {
               case Color::RED:
                  mat = new material(vec4(1, 0, 0, 1));
                  modelToWorld.translate(-(boardsize.x() * 0.5f), boardsize.y() * 2, -0);
                  break;
               case Color::GREEN:
                  mat = new material(vec4(0, 1, 0, 1));
                  modelToWorld.translate(boardsize.x()  * 0.5f, boardsize.y() * 2, 0);
                  break;
               case Color::BLUE:
                  mat = new material(vec4(0, 0, 1, 1));
                  modelToWorld.translate(0, boardsize.y() * 2, boardsize.x() * 0.5f);
                  break;
               case Color::YELLOW:
                  mat = new material(vec4(1, 1, 0, 1));
                  modelToWorld.translate(0, boardsize.y() * 2, -(boardsize.x() * 0.5f));
                  break;
                  default:
                     break;
               }

               Player *player = new Player(3.0f, 1.0f, *mat, (Color)i, modelToWorld); //Chuck: assign a transform here to pass to player, the player does not need to know board dimension
               
               world->addRigidBody(player->GetRigidBody());
               
               btTransform localConstr;
               localConstr.setIdentity();
               btVector3 playerCOM = player->GetRigidBody()->getCenterOfMassPosition();               
               //localConstr.setOrigin(btVector3(-(playerCOM.getX()), -(playerCOM.getY()), -(playerCOM.getZ())));
               btGeneric6DofConstraint* constr = new btGeneric6DofConstraint((*player->GetRigidBody()), localConstr, true);//(*player->GetRigidBody()),*board->GetRigidBody(), localConstr, localConstr2, false
                              
               world->addConstraint(constr);
               constr->setLinearLowerLimit(btVector3(-boardsize.x() - (playerCOM.getX()) - 200, -100 - playerCOM.getY(), -boardsize.x() - (playerCOM.getZ()) - 200));
               constr->setLinearUpperLimit(btVector3(boardsize.x() - (playerCOM.getX()) + 200, 10, boardsize.x() - (playerCOM.getZ()) + 200));
               constr->setAngularLowerLimit(btVector3(-SIMD_PI * 0.25, 0, -SIMD_PI * 0.25));
               constr->setAngularUpperLimit(btVector3(SIMD_PI * 0.25, 0, SIMD_PI * 0.25));
               //constr->setAngularLowerLimit(btVector3(0, 0, 0));
               //constr->setAngularUpperLimit(btVector3(0, 0, 0));
               
              app_scene->add_child(player->GetNode());
              app_scene->add_mesh_instance(player->GetMesh());
               
               players.push_back(player);
            }
         }

         /// this is called to draw the world
         void draw_world(int x, int y, int w, int h) {
            
            acquireInputs();
            
            world->stepSimulation(1.0f/60);            
            
            checkPLayersStatus();

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
