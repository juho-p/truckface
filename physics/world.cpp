#include "world.hpp"

#include <btBulletDynamicsCommon.h>

#include <glm/gtc/type_ptr.hpp>

#include <thread>
#include <mutex>

using std::vector;

namespace physics {

inline glm::mat4 transform_to_matrix(btTransform transform) {
    glm::mat4 res;
    transform.getOpenGLMatrix(glm::value_ptr(res));
    return res;
}

struct MotionState : public btMotionState {
    MotionState(btTransform t, ObjectId id, World* w) : transform(t), object_id(id), world(w) {}
    virtual ~MotionState() {}

    btTransform transform;
    ObjectId object_id;
    World* world;

    virtual void getWorldTransform(btTransform& worldTrans) const {
        worldTrans = transform;
    }
    virtual void setWorldTransform(const btTransform& worldTrans) {
        transform = worldTrans;
        world->update_change(object_id, transform_to_matrix(transform));
    }
};

struct Box {
    btBoxShape shape;
    MotionState state;
};

enum WorldStatus {
    Idle, Running, Stopping
};

struct WorldRes {
    vector<unique_ptr<Box>> boxes;
    vector<unique_ptr<btRigidBody>> bodies;

    unique_ptr<btBroadphaseInterface> broadphase;
    unique_ptr<btCollisionDispatcher> dispatcher;
    unique_ptr<btDefaultCollisionConfiguration> collision_config;
    unique_ptr<btSequentialImpulseConstraintSolver> solver;
    unique_ptr<btDiscreteDynamicsWorld> world;

    std::mutex changes_mutex;
    std::thread thread;
    WorldStatus status;

    WorldRes() {
        broadphase.reset(new btDbvtBroadphase());
        collision_config.reset(new btDefaultCollisionConfiguration());
        dispatcher.reset(new btCollisionDispatcher(collision_config.get()));
        solver.reset(new btSequentialImpulseConstraintSolver());
        world.reset(new btDiscreteDynamicsWorld(
                    dispatcher.get(), broadphase.get(), solver.get(),
                    collision_config.get()));
        status = Idle;
    }
};

World::World() {
    this->res = new WorldRes;

    res->world->setGravity(btVector3(0, -10, 0));
}

World::~World() {
    stop();
    delete res;
}

void World::add_cube(ObjectId id, glm::mat4 transform, float mass, float x, float y, float z) {
    btTransform trans;
    trans.setFromOpenGLMatrix(glm::value_ptr(transform));
    unique_ptr<Box> box(new Box{
            btBoxShape(btVector3(x, y, z)),
            MotionState{trans, id, this} });

    btVector3 inertia(0,0,0);
    box->shape.calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo body_ci(
            mass, &box->state, &box->shape, inertia);
    unique_ptr<btRigidBody> body(new btRigidBody(body_ci));

    res->boxes.push_back(move(box));

    res->world->addRigidBody(body.get());
    res->bodies.push_back(move(body));
}


std::map<ObjectId, glm::mat4> World::get_and_reset_changes() {
    std::map<ObjectId, glm::mat4> result;
    std::lock_guard<std::mutex> lock(res->changes_mutex);
    this->changes.swap(result);
    return result;
}

void World::update_change(ObjectId id, const glm::mat4& change) {
    std::lock_guard<std::mutex> lock(res->changes_mutex);
    changes[id] = change;
}

void World::single_step() {
    // step single fixed time
    this->res->world->stepSimulation(1.0/60.0, 0);
}

void World::run() {
    WorldStatus status = res->status;
    if (status == Running) {
        // thread already running, do nothing
        return;
    } else if (status == Stopping) {
        // wait for thread to stop before starting it again
        res->thread.join();
    }

    // start the thread
    assert(res->status == Idle);
    res->status = Running;
    res->thread = std::thread([this]() {
        // our goal is to step once per 1/60 secs
        // if the simulation is too slow, then tough luck
        // excessive time is spent in sleep
        const auto step_time = std::chrono::milliseconds(1000/60);
        while (this->res->status == Running) {
            auto start_time = std::chrono::steady_clock::now();
            single_step();

            if (this->res->status == Running) {
                std::this_thread::sleep_until(start_time + step_time);
            }
        }
        assert(this->res->status == Stopping);
        this->res->status = Idle;
    });
}

void World::pause() {
    if (res->status == Running) {
        res->status = Stopping;
    }
}

void World::stop() {
    if (res->status == Running) {
        pause();
        res->thread.join();
        assert(res->status == Idle);
    }
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
