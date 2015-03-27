#include "world.hpp"

#include <btBulletDynamicsCommon.h>

#include <glm/gtc/type_ptr.hpp>

#include <thread>
#include <mutex>
#include <atomic>

#include <unordered_map>

#include "../util/task_list.hpp"

namespace physics {

enum Status_ {
    Idle, Running, Stopping
};
typedef std::atomic<Status_> Status;

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

struct PObj {
    virtual ~PObj() {};
    virtual void remove_from_world(btDiscreteDynamicsWorld* world) = 0;
};

struct Cube : public PObj {
    unique_ptr<btBoxShape> shape;
    unique_ptr<MotionState> state;
    unique_ptr<btRigidBody> body;

    virtual ~Cube() {}
    virtual void remove_from_world(btDiscreteDynamicsWorld* world) {
        world->removeRigidBody(body.get());
    }
};
struct Car : public PObj {
    btRaycastVehicle::btVehicleTuning tuning;
    unique_ptr<MotionState> state;
    unique_ptr<btCollisionShape> chassis_shape;
    unique_ptr<btCompoundShape> compound;
    unique_ptr<btRigidBody> chassis;
    unique_ptr<btVehicleRaycaster> ray_caster;
    unique_ptr<btRaycastVehicle> vehicle;

    virtual ~Car() {}
    virtual void remove_from_world(btDiscreteDynamicsWorld* world) {
        world->removeVehicle(vehicle.get());
        world->removeRigidBody(chassis.get());
    }
};

struct WorldRes {
    std::unordered_map<ObjectId, unique_ptr<PObj>> objects;

    unique_ptr<btBroadphaseInterface> broadphase;
    unique_ptr<btCollisionDispatcher> dispatcher;
    unique_ptr<btDefaultCollisionConfiguration> collision_config;
    unique_ptr<btSequentialImpulseConstraintSolver> solver;
    unique_ptr<btDiscreteDynamicsWorld> world;

    std::mutex changes_mutex;
    std::thread thread;
    Status thread_status;
    util::TaskList tasks;

    WorldRes() {
        broadphase.reset(new btDbvtBroadphase());
        collision_config.reset(new btDefaultCollisionConfiguration());
        dispatcher.reset(new btCollisionDispatcher(collision_config.get()));
        solver.reset(new btSequentialImpulseConstraintSolver());
        world.reset(new btDiscreteDynamicsWorld(
                    dispatcher.get(), broadphase.get(), solver.get(),
                    collision_config.get()));
        thread_status = Idle;
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
        unique_ptr<Cube> cube{new Cube};
        btTransform trans;
        trans.setFromOpenGLMatrix(glm::value_ptr(transform));

        cube->state.reset(new MotionState(trans, id, this));

        cube->shape.reset(new btBoxShape(btVector3(x, y, z)));
        btVector3 inertia(0,0,0);
        cube->shape->calculateLocalInertia(mass, inertia);

        cube->body.reset(new btRigidBody(
                    btRigidBody::btRigidBodyConstructionInfo(
                        mass, cube->state.get(), cube->shape.get(), inertia)));

        res->world->addRigidBody(cube->body.get());
        res->objects[id] = move(cube);
    });
}

void World::add_car(ObjectId id, glm::mat4 transform) {
    res->tasks.add([=]() {
        const double mass = 800.0;
        const double wheel_width = 0.4;
        const double wheel_radius = 1.5;
        const double connection_height = 1.2;
        const btVector3 wheel_direction(0,-1,0);
        const btVector3 wheel_axle(-1,0,0);
        const double suspension_rest_len = 0.6;

        btTransform trans;
        trans.setFromOpenGLMatrix(glm::value_ptr(transform));

        unique_ptr<Car> car{new Car};

        car->state.reset(new MotionState(trans, id, this));
        car->chassis_shape.reset(new btBoxShape(btVector3(1.f,0.5f, 2.0f)));
        car->compound.reset(new btCompoundShape());
        btTransform local_trans;
        local_trans.setIdentity();
        local_trans.setOrigin(btVector3(0,1,0));
        car->compound->addChildShape(local_trans, car->chassis_shape.get());
        btVector3 inertia(0,0,0);
        car->compound->calculateLocalInertia(mass,inertia);
        car->chassis.reset(new btRigidBody(
                btRigidBody::btRigidBodyConstructionInfo(
                    mass, car->state.get(), car->compound.get(), inertia)));
        res->world->addRigidBody(car->chassis.get());

        car->ray_caster.reset(new btDefaultVehicleRaycaster(res->world.get()));
        car->vehicle.reset(new btRaycastVehicle(car->tuning, car->chassis.get(), car->ray_caster.get()));
		car->chassis->setActivationState(DISABLE_DEACTIVATION);
        res->world->addVehicle(car->vehicle.get());

        car->vehicle->addWheel(btVector3(1-(0.3*wheel_width), connection_height, 2-wheel_radius), wheel_direction, wheel_axle, suspension_rest_len, wheel_radius, car->tuning, true);
        car->vehicle->addWheel(btVector3(-1+(0.3*wheel_width), connection_height, 2-wheel_radius), wheel_direction, wheel_axle, suspension_rest_len, wheel_radius, car->tuning, true);
        car->vehicle->addWheel(btVector3(1-(0.3*wheel_width), connection_height, -2+wheel_radius), wheel_direction, wheel_axle, suspension_rest_len, wheel_radius, car->tuning, false);
        car->vehicle->addWheel(btVector3(-1+(0.3*wheel_width), connection_height, -2+wheel_radius), wheel_direction, wheel_axle, suspension_rest_len, wheel_radius, car->tuning, false);

        float wheelFriction = 1000;
        float suspensionStiffness = 20.f;
        float suspensionDamping = 2.3f;
        float suspensionCompression = 4.4f;
        float rollInfluence = 0.1f;
        for (int i=0;i<car->vehicle->getNumWheels();i++)
        {
            btWheelInfo& wheel = car->vehicle->getWheelInfo(i);
            wheel.m_suspensionStiffness = suspensionStiffness;
            wheel.m_wheelsDampingRelaxation = suspensionDamping;
            wheel.m_wheelsDampingCompression = suspensionCompression;
            wheel.m_frictionSlip = wheelFriction;
            wheel.m_rollInfluence = rollInfluence;
        }
        res->objects[id] = move(car);
    });
}

void World::engine(ObjectId cid, bool run) {
    res->tasks.add([=]() {
        float force = run ? 100.0 : 0.0;
        auto it = res->objects.find(cid);
        if (it != res->objects.end()) {
            cout << "engine set to " << run << " for " << cid << endl;
            auto car = dynamic_cast<Car*>(it->second.get());
            auto veh = car->vehicle.get();
            veh->applyEngineForce(force, 2);
            veh->applyEngineForce(force, 3);
        }
    });
}
void World::steer(ObjectId cid, float val) {
    res->tasks.add([=]() {
        auto it = res->objects.find(cid);
        if (it != res->objects.end()) {
            cout << "seer set to " << val << " for " << cid << endl;
            auto car = dynamic_cast<Car*>(it->second.get());
            auto veh = car->vehicle.get();
            veh->setSteeringValue(val, 0);
            veh->setSteeringValue(val, 1);
        }
    });
}

void World::remove(ObjectId id) {
    res->tasks.add([=]() {
        auto it = res->objects.find(id);
        if (it != res->objects.end()) {
            auto obj = it->second.get();
            obj->remove_from_world(res->world.get());
            res->objects.erase(it);
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
    assert(res->thread_status == Idle);
    single_step_();
}

void World::single_step_() {
    res->tasks.run();
    // step single fixed time
    this->res->world->stepSimulation(1.0/60.0, 0);
}

void World::run() {
    auto status = res->thread_status.load();
    if (status == Running) {
        // thread already running, do nothing
        return;
    } else if (status == Stopping) {
        // wait for thread to stop before starting it again
        this->stop();
    }

    // start the thread
    assert(res->thread_status == Idle);
    res->thread_status.store(Running);

    res->thread = std::thread([this]() {
        // our goal is to step once per 1/60 secs
        // if the simulation is too slow, then tough luck
        // excessive time is spent in sleep
        const auto step_time = std::chrono::milliseconds(1000/60);
        while (res->thread_status == Running) {
            auto start_time = std::chrono::steady_clock::now();
            single_step_();

            std::this_thread::sleep_until(start_time + step_time);
        }

        auto expected = Stopping;
        bool ok = res->thread_status.compare_exchange_weak(expected, Idle);
        assert(ok);
    });
}

void World::stop() {
    if (res->thread_status != Idle) {
        auto expected = Running;
        res->thread_status.compare_exchange_weak(expected, Stopping);
        res->thread.join();
        assert(res->thread_status == Idle);
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
