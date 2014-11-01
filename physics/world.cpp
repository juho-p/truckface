#include "world.hpp"

#include <btBulletDynamicsCommon.h>

#include <glm/gtc/type_ptr.hpp>

#include <thread>
#include <mutex>

#include <unordered_map>

#include "../util/task_list.hpp"
#include "../util/threaded.hpp"

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

struct Cube {
    unique_ptr<btBoxShape> shape;
    unique_ptr<MotionState> state;
    unique_ptr<btRigidBody> body;
};

struct WorldRes {
    std::unordered_map<ObjectId, Cube> cubes;

    unique_ptr<btBroadphaseInterface> broadphase;
    unique_ptr<btCollisionDispatcher> dispatcher;
    unique_ptr<btDefaultCollisionConfiguration> collision_config;
    unique_ptr<btSequentialImpulseConstraintSolver> solver;
    unique_ptr<btDiscreteDynamicsWorld> world;

    std::mutex changes_mutex;
    std::thread thread;
    util::threaded::Status thread_status;
    util::TaskList tasks;

    WorldRes() {
        broadphase.reset(new btDbvtBroadphase());
        collision_config.reset(new btDefaultCollisionConfiguration());
        dispatcher.reset(new btCollisionDispatcher(collision_config.get()));
        solver.reset(new btSequentialImpulseConstraintSolver());
        world.reset(new btDiscreteDynamicsWorld(
                    dispatcher.get(), broadphase.get(), solver.get(),
                    collision_config.get()));
        thread_status = util::threaded::Idle;
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
    res->tasks.add([=]() {
        btTransform trans;
        trans.setFromOpenGLMatrix(glm::value_ptr(transform));

        unique_ptr<MotionState> state(new MotionState(trans, id, this));

        unique_ptr<btBoxShape> shape(new btBoxShape(btVector3(x, y, z)));
        btVector3 inertia(0,0,0);
        shape->calculateLocalInertia(mass, inertia);

        unique_ptr<btRigidBody> body(new btRigidBody(
                    btRigidBody::btRigidBodyConstructionInfo(
                        mass, state.get(), shape.get(), inertia)));

        auto it = res->cubes.emplace(
                std::make_pair(id,
                    Cube{ move(shape), move(state), move(body) }));
        res->world->addRigidBody(it.first->second.body.get());
    });
}

void World::remove(ObjectId id) {
    res->tasks.add([=]() {
        auto cube = res->cubes.find(id);
        if (cube != res->cubes.end()) {
            res->world->removeRigidBody(cube->second.body.get());
            res->cubes.erase(cube);
        }
    });
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
    // only allow calling this when the thread is not running
    assert(res->thread_status == util::threaded::Idle);
    single_step_();
}

void World::single_step_() {
    res->tasks.run();
    // step single fixed time
    this->res->world->stepSimulation(1.0/60.0, 0);
}

void World::run() {
    if (!util::threaded::pre_run(res)) return;

    res->thread = std::thread([this]() {
        // our goal is to step once per 1/60 secs
        // if the simulation is too slow, then tough luck
        // excessive time is spent in sleep
        const auto step_time = std::chrono::milliseconds(1000/60);
        while (util::threaded::is_running(res)) {
            auto start_time = std::chrono::steady_clock::now();
            single_step_();

            std::this_thread::sleep_until(start_time + step_time);
        }

        util::threaded::post_run(this->res);
    });
}

void World::stop() {
    util::threaded::stop(res);
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
