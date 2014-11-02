////////////////////////////////////////////////////////////////////////////////
//
// (C) Andrea Castegnaro 2014
//
//
//
#pragma once

#define DEBUG_EN 1

#include "Player.h"
#include "Game.h"
#include "Board.h"
#include "AI.h"
#include "Joystick.h"
#include "Clock.h"

namespace octet {
  /// Scene using bullet for physics effects.
   static const char keyboardset[16] = {

      key_up, key_left, key_down, key_right, 
      'W', 'A', 'S', 'D',
      'T', 'F', 'G', 'H',
      'I', 'J', 'K', 'L'

      };

   class example_physics : public app {
   
      Joystick* joystick = nullptr;

      int num_players;
      enum {MaxPLayers = 4};

      ref<Game> gs;
      unsigned gameWinner = -1;

      const int gravity = -80;
      
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
      ref<AI> ai;
      ref<Board> board;
      
      //Chuck: adding reference to the camera
      ref<camera_instance> scenecamera;

      //UI elements
      ref<text_overlay> overlay;
      dynarray<ref<mesh_text>> texts;

      void GameReset(){
         ResetPhysics();
         InitPhysics();
         joystick->ShutDown();
         delete joystick;
         app_init();
      }

      void ResetPlayer(int playerIdx){

         btScalar boardRadius = board->GetRadius();
         btScalar boardhalfheight = board->GetHalfHeight();

         mat4t modelToWorld;
         modelToWorld.loadIdentity();

         players[playerIdx]->GetRigidBody()->clearForces();
         players[playerIdx]->SetState(PlayerState::Ingame);

         switch (players[playerIdx]->GetColor())
         {
         case Color::RED:
            modelToWorld.translate(-(boardRadius * 0.5f), boardhalfheight * 20, 0);
            break;
         case Color::GREEN:
            modelToWorld.translate(boardRadius * 0.5f, boardhalfheight * 20, 0);// var->GetRigidBody()->translate(btVector3(boardRadius * 0.5f, boardhalfheight * 2, 0));
            break;
         case Color::BLUE:
            modelToWorld.translate(0, boardhalfheight * 20, boardRadius * 0.5f);// var->GetRigidBody()->translate(btVector3(0, boardhalfheight * 2, boardRadius * 0.5f));
            break;
         case Color::YELLOW:
            modelToWorld.translate(0, boardhalfheight * 20, -(boardRadius * 0.5f));// var->GetRigidBody()->translate(btVector3(0, boardhalfheight * 2, -(boardRadius * 0.5f)));
            break;
         default:
            break;
         }

         btTransform trans(get_btMatrix3x3(modelToWorld), get_btVector3(modelToWorld[3].xyz()));
         players[playerIdx]->GetRigidBody()->setWorldTransform(trans);
         players[playerIdx]->GetRigidBody()->setLinearVelocity(btVector3(0,0,0));
         players[playerIdx]->GetRigidBody()->setAngularVelocity(btVector3(0,0,0));
         players[playerIdx]->GetRigidBody()->applyCentralImpulse(btVector3(0,-10,0)); // SetTransform(trans); //translate(btVector3(-(boardRadius * 0.5f), boardhalfheight * 2, 0));

      }
      //Chuck: It will used more for debug reasons!
      void ResetPlayers(){

         gs->Reset();

         btScalar boardRadius = board->GetRadius();
         btScalar boardhalfheight = board->GetHalfHeight();

         mat4t modelToWorld;

         for each (auto var in players)
         {
            modelToWorld.loadIdentity();

            var->GetRigidBody()->clearForces();
            var->SetState(PlayerState::Ingame);
            var->SetLife(4);

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
            var->GetRigidBody()->setWorldTransform(trans);
            var->GetRigidBody()->setLinearVelocity(btVector3(0, 0, 0)); // SetTransform(trans); //translate(btVector3(-(boardRadius * 0.5f), boardhalfheight * 2, 0));

         }
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

      void AcquireInputs(){

         if (is_key_down(key_esc)) exit(1);

         if (gs->GetState() == GameState::END || DEBUG_EN){
            if (is_key_down(key_space))
            {
               ResetPlayers();
            }
         }
         if (gs->GetState() == GameState::PLAY){

            unsigned offsetKey = 0;
            for (unsigned i = 0; i < players.size(); i++)
            {
               //Chuck: Dealing movement
               if (players[i]->GetState() == PlayerState::Ingame && !players[i]->GetAiEnabled()){

                  if (i < joystick->GetNumberOfDevicesFound()){
                     if (joystick->AcquireInputData(i)){
                        DIJOYSTATE* joyInput = joystick->GetCurrentState();
                        players[i]->ApplyCentralForce(btVector3(joyInput->lX, 0, joyInput->lY));
                        players[i]->ApplyPowerUps(joyInput->rgbButtons);
                     }
                  }
                  else{
                     if (DEBUG_EN) offsetKey = i * 4;
                     btVector3 physics_vector(0, 0, 0);
                     if (is_key_down(keyboardset[0 + offsetKey])){
                        physics_vector += (btVector3(0, 0, -120));
                     }
                     else if (is_key_down(keyboardset[2 + offsetKey])){
                        physics_vector += (btVector3(0, 0, 120));
                     }

                     if (is_key_down(keyboardset[1 + offsetKey])){
                        physics_vector += (btVector3(-120, 0, 0));
                     }
                     else if (is_key_down(keyboardset[3 + offsetKey])){
                        physics_vector += (btVector3(120, 0, 0));
                     }

                     players[i]->ApplyCentralForce(physics_vector);
                  
                  }
               }
               //AI Decision
               else
               {
                  ai->MovePlayer(players, i);
               }
            }
         }
      }

      void CheckPLayersStatus(){

         btScalar boardRadius = board->GetRadius();
         btScalar boardHeigth = board->GetHalfHeight();
         btScalar active_lower_radius = boardRadius * (1 + 0.1); //Chuck: I write like this to give them a sens
         btScalar active_upper_radius = boardRadius *(1 + 0.1) + 0.2f;
         btScalar active_height_lower = boardHeigth * 2;
         btScalar active_height_upper = boardHeigth * 2 + 0.2f;
         btScalar dead_radius = boardRadius * 2.5f;
         btScalar dead_heigth = -20.0f;

         for (unsigned i = 0; i < players.size(); i++)
         {
            
            //Chuck: Control position respect to the arena
            btRigidBody* currBody = players[i]->GetRigidBody();
            btVector3 currPLayer = currBody->getCenterOfMassPosition();
            
            btVector3 currPLayerXZ(currPLayer.getX(), 0, currPLayer.getZ());
            btScalar playY = currPLayer.getY();
            
            btScalar distanceToBoard = currPLayerXZ.distance(btVector3(0,0,0));
            switch (players[i]->GetState())
            {
               case PlayerState::Ingame:
                  if (distanceToBoard >= boardRadius + active_upper_radius || math::abs(playY) > active_height_upper){
                     players[i]->SetState(PlayerState::Inactive);
                     if (DEBUG_EN){
                        printf("Player %s Inactive \n", players[i]->GetColorString());
                     }
                  }
                  break;
               case PlayerState::Inactive:
                  if ((math::abs(playY) <= active_height_lower) && distanceToBoard <= active_lower_radius){
                     players[i]->SetState(PlayerState::Ingame);
                     if (DEBUG_EN){
                        printf("Player %s InGame \n", players[i]->GetColorString());
                     }
                  }
                  else if (playY <= dead_heigth || distanceToBoard >= dead_radius){
                     players[i]->DecreaseLife();
                     players[i]->SetState(PlayerState::KO);
                     if (DEBUG_EN){
                        printf("Player %s KO \n", players[i]->GetColorString());
                     }
                  }
                  break;
               case PlayerState::KO:
                  if (!(players[i]->IsLifeEnd())){
                     ResetPlayer(i);
                     players[i]->SetState(PlayerState::Respawing);
                     if (DEBUG_EN){
                        printf("Player %s Respanwing \n", players[i]->GetColorString());
                     }
                  }
                  else{
                     players[i]->SetState(PlayerState::Dead);
                     if (DEBUG_EN){
                        printf("Player %s Dead \n", players[i]->GetColorString());
                     }
                  }
               break;
               case PlayerState::Respawing:
                  if (playY <= boardHeigth * 2){
                     players[i]->SetState(PlayerState::Ingame);
                     if (DEBUG_EN){
                        printf("Player %s Ingame after respawn \n", players[i]->GetColorString());
                     }
                  }
                  break;
               default:
                  break;
            }
            
            players[i]->CheckPowerUps();

         }
      }
   
      void CheckGameStatus(){
         
         int num_winner;
         int winner;

         gs->CheckGameStatus(players);
      }

      void SetPlayersToTheWorld(){
         for (unsigned i = 0; i != players.size(); i++)
         {
            players[i]->SetObjectToTheWorld();
         }
      }
      
      void RenderScene(int& vx, int& vy){
         app_scene->update(1.0f / 30);
         get_viewport_size(vx, vy);
         app_scene->begin_render(vx, vy);
         // draw the scene
         app_scene->render((float)vx / vy);
      }

      void RenderText(int vx, int vy){

         for (unsigned i = 0; i < texts.size(); i++)
         {
            if (i < players.size())
            {
               texts[i]->clear();

               texts[i]->format("%s", players[i]->GetInfoString());

               texts[i]->update();
            }
         }

         gs->RenderText();
         
         // draw the text overlay
         overlay->render(vx, vy);
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

            joystick = new Joystick();
            joystick->InitInputDevice(this);

            num_players = joystick->GetNumberOfDevicesFound();

            //Chuck: I will activate the AI if not device found
            if (num_players == 0) num_players = 4;
            else (num_players += 1);

            ai = new AI();

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
            
            // create the overlay
            overlay = new text_overlay();
            bitmap_font* font = overlay->get_default_font();

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

               bool aIcontrolled = i >= joystick->GetNumberOfDevicesFound();
               Player *player = new Player(3.0f, 1.0f, *mat, (Color)i, modelToWorld, aIcontrolled, 4); //Chuck: assign a transform here to pass to player, the player does not need to know board dimension
               
               world->addRigidBody(player->GetRigidBody());
               
               btTransform localConstr;
               localConstr.setIdentity();
               btVector3 playerCOM = player->GetRigidBody()->getCenterOfMassPosition();               
               //localConstr.setOrigin(btVector3(-(playerCOM.getX()), -(playerCOM.getY()), -(playerCOM.getZ())));
               btGeneric6DofConstraint* constr = new btGeneric6DofConstraint((*player->GetRigidBody()), localConstr, true);//(*player->GetRigidBody()),*board->GetRigidBody(), localConstr, localConstr2, false
                              
               world->addConstraint(constr);
               constr->setLinearLowerLimit(btVector3(-10000,-10000,-10000));
               constr->setLinearUpperLimit(btVector3(10000, 10000, 10000));
               /*constr->setLinearLowerLimit(btVector3(-boardsize.x() - (playerCOM.getX()) - 200, -100 - playerCOM.getY(), -boardsize.x() - (playerCOM.getZ()) - 200));
               constr->setLinearUpperLimit(btVector3(boardsize.x() - (playerCOM.getX()) + 200, 10, boardsize.x() - (playerCOM.getZ()) + 200));*/
               constr->setAngularLowerLimit(btVector3(-SIMD_PI * 0.25, 0, -SIMD_PI * 0.25));
               constr->setAngularUpperLimit(btVector3(SIMD_PI * 0.25, 0, SIMD_PI * 0.25));
               //constr->setAngularLowerLimit(btVector3(0, 0, 0));
               //constr->setAngularUpperLimit(btVector3(0, 0, 0));
               
               app_scene->add_child(player->GetNode());
               app_scene->add_mesh_instance(player->GetMesh());
               
               players.push_back(player);

               //Chuck adding UI information: after the first two player make some spaces
               float offset = i >= 2 ? 250.0f : 0.0f;
               aabb bb(vec3(-500.0f + 250.0f * i + offset, 100.0f, 0), vec3(120, 200, 0));
               mesh_text* curr_text = new mesh_text(font, "", &bb);
               
               overlay->add_mesh_text(curr_text);

               texts.push_back(curr_text);
                              
            }

            gs = new Game(GameState::PLAY, font);
            overlay->add_mesh_text(gs->GetWinnerText());
            texts.push_back(gs->GetWinnerText());

         }

         /// this is called to draw the world
         void draw_world(int x, int y, int w, int h) {
            
            world->stepSimulation(1.0f/30, 10);            
            
            AcquireInputs();

            CheckPLayersStatus();

            CheckGameStatus();

            SetPlayersToTheWorld();
            
            int vx = 0, vy = 0;
            
            RenderScene(vx,vy);
            
            RenderText(vx,vy);

         }
      
      };
}
