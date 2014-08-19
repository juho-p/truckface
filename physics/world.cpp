#include "world.hpp"

#include <btBulletDynamicsCommon.h>

#include <glm/gtc/type_ptr.hpp>

#include <forward_list>

using std::vector;

namespace physics {

inline glm::mat4 transform_to_matrix(btTransform transform) {
    glm::mat4 res;
    transform.getOpenGLMatrix(glm::value_ptr(res));
    return res;
}

struct MotionState : public btMotionState {
    MotionState() {}
    MotionState(btTransform t) : transform(t) {}
    virtual ~MotionState() {}

    btTransform transform;
    std::function<void(glm::mat4)> callback;

    virtual void getWorldTransform(btTransform& worldTrans) const {
        worldTrans = transform;
    }
    virtual void setWorldTransform(const btTransform& worldTrans) {
        transform = worldTrans;
        if (callback) {
            callback(transform_to_matrix(transform));
        }
    }
};

struct Box {
    btBoxShape shape;
    MotionState state;
};

struct WorldRes {
    unique_ptr<btBoxShape> ground;
    MotionState ground_state;
    vector<unique_ptr<Box>> boxes;
    vector<unique_ptr<btRigidBody>> bodies;

    unique_ptr<btBroadphaseInterface> broadphase;
    unique_ptr<btCollisionDispatcher> dispatcher;
    unique_ptr<btDefaultCollisionConfiguration> collision_config;
    unique_ptr<btSequentialImpulseConstraintSolver> solver;
    unique_ptr<btDiscreteDynamicsWorld> world;

    WorldRes() {
        broadphase.reset(new btDbvtBroadphase());
        collision_config.reset(new btDefaultCollisionConfiguration());
        dispatcher.reset(new btCollisionDispatcher(collision_config.get()));
        solver.reset(new btSequentialImpulseConstraintSolver());
        world.reset(new btDiscreteDynamicsWorld(
                    dispatcher.get(), broadphase.get(), solver.get(),
                    collision_config.get()));
        ground.reset(new btBoxShape(btVector3(50, 50, 50)));
    }
};

World::World() {
    this->res = new WorldRes;

    res->world->setGravity(btVector3(0, -10, 0));

    res->ground_state.transform.setIdentity();
    res->ground_state.transform.setOrigin(btVector3(0, -50, 0));

    // static ground
    btScalar ground_mass(0.0);
    btVector3 local_inertia(0.0, 0.0, 0.0);

    unique_ptr<btRigidBody> body(new btRigidBody(
                btRigidBody::btRigidBodyConstructionInfo(
                    ground_mass, &res->ground_state,
                    res->ground.get(), local_inertia)));
    res->world->addRigidBody(body.get());
    res->bodies.push_back(move(body));
}

World::~World() {
    delete res;
}

void World::add_cube(glm::vec3 pos, float scale,
        std::function<void(glm::mat4)> transform_changed) {

    btScalar mass(scale*scale*scale);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    unique_ptr<Box> box(new Box{
            btBoxShape(btVector3(scale, scale, scale)),
            MotionState{transform} });
    box->state.callback = transform_changed;

    btVector3 inertia(0,0,0);
    box->shape.calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo body_ci(
            mass, &box->state, &box->shape, inertia);
    unique_ptr<btRigidBody> body(new btRigidBody(body_ci));

    res->boxes.push_back(move(box));

    res->world->addRigidBody(body.get());
    res->bodies.push_back(move(body));
}

void World::step() {
    res->world->stepSimulation(1.0f/60.0f, 10);
}

void World::printworld() {
    cout << "//printworld\n";
    const btCollisionObjectArray& arr = res->world->getCollisionObjectArray();
    for (int i = 0; i < arr.size(); i++) {
        const auto& body = dynamic_cast<btRigidBody*>(arr[i]);
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        cout << " " << (body->isStaticObject() ? string("static obj") : string("obj")) << endl;
        auto o = trans.getOrigin();
        cout << "  loc: " << o.getX() << ' ' << o.getY() << ' ' << o.getZ() << endl;
        cout << "  mass " << body->getInvMass() << endl;
        if (body->isActive())
            cout << "  ACTIVE" << endl;
    }
    cout << "\\\\printworld\n" << endl;
}

}
