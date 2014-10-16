////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include "Player.h"
#include "Board.h"

namespace octet {
  /// Scene using bullet for physics effects.
  class example_physics : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

    btDefaultCollisionConfiguration config;       /// setup for the world
    btCollisionDispatcher *dispatcher;            /// handler for collisions between objects
    btDbvtBroadphase *broadphase;                 /// handler for broadphase (rough) collision
    btSequentialImpulseConstraintSolver *solver;  /// handler to resolve collisions
    btDiscreteDynamicsWorld *world;               /// physics world, contains rigid bodies

    dynarray<btRigidBody*> rigid_bodies;
    dynarray<scene_node*> nodes;
    
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
        rigid_bodies.push_back(rigidBody);

        mesh_sphere *meshsphere = new mesh_sphere(vec3(0, 0, 0), size);
        scene_node *node = new scene_node(modelToWorld, atom_);
        nodes.push_back(node);

        app_scene->add_child(node);
        app_scene->add_mesh_instance(new mesh_instance(node, meshsphere, mat));

    }

   void add_box(mat4t_in modelToWorld, vec3_in size, material *mat, bool is_dynamic=true) {

      btMatrix3x3 matrix(get_btMatrix3x3(modelToWorld));
      btVector3 pos(get_btVector3(modelToWorld[3].xyz()));

      btCollisionShape *shape = new btBoxShape(get_btVector3(size));

      btTransform transform(matrix, pos);

      btDefaultMotionState *motionState = new btDefaultMotionState(transform);
      btScalar mass = is_dynamic ? 1.0f : 0.0f;
      btVector3 inertiaTensor;
   
      shape->calculateLocalInertia(mass, inertiaTensor);
    
      btRigidBody * rigid_body = new btRigidBody(mass, motionState, shape, inertiaTensor);
      world->addRigidBody(rigid_body);
      rigid_bodies.push_back(rigid_body);

      mesh_box *box = new mesh_box(size);
      scene_node *node = new scene_node(modelToWorld, atom_);
      nodes.push_back(node);

      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, mat));
    }
  public:
    /// this is called when we construct the class before everything is initialised.
    example_physics(int argc, char **argv) : app(argc, argv) {
      dispatcher = new btCollisionDispatcher(&config);
      broadphase = new btDbvtBroadphase();
      solver = new btSequentialImpulseConstraintSolver();
      world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, &config);
    }

    ~example_physics() {
      delete world;
      delete solver;
      delete broadphase;
      delete dispatcher;
    }

    /// this is called once OpenGL is initialized
    void app_init() {

      app_scene =  new visual_scene();

      app_scene->create_default_camera_and_lights();
      scenecameranode = app_scene->get_camera_instance(0)->get_node();

      //Chuck: camera is fixed, changing the parameters in order to have camera looking at the platform along Z-axis
      scenecameranode->access_nodeToParent().rotateX(-30);
      scenecameranode->access_nodeToParent().translate(0,20,20);

      // add the ground (as a static object)
      board = new Board(20.0f,2.0f);
      world->addRigidBody(board->GetRigidBody());
      rigid_bodies.push_back(board->GetRigidBody());
      nodes.push_back(board->GetNode());
      app_scene->add_child(board->GetNode());
      app_scene->add_mesh_instance(board->GetMesh());

      Player *player = new Player(1.0f, 0.5f, Color::RED);
      world->addRigidBody(player->GetRigidBody());
      rigid_bodies.push_back(player->GetRigidBody());
      nodes.push_back(player->GetNode());
      app_scene->add_child(player->GetNode());
      app_scene->add_mesh_instance(player->GetMesh());

   }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      world->stepSimulation(1.0f/30);

      for (unsigned i = 0; i != rigid_bodies.size(); ++i) {
        btRigidBody *rigid_body = rigid_bodies[i];
        btQuaternion btq = rigid_body->getOrientation();
        btVector3 pos = rigid_body->getCenterOfMassPosition();
        quat q(btq[0], btq[1], btq[2], btq[3]);
        mat4t modelToWorld = q;
        modelToWorld[3] = vec4(pos[0], pos[1], pos[2], 1);
        nodes[i]->access_nodeToParent() = modelToWorld;
      }

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
