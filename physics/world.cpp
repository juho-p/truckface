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
struct Car {
    btRaycastVehicle::btVehicleTuning tuning;
    unique_ptr<MotionState> state;
    unique_ptr<btCollisionShape> chassis_shape;
    unique_ptr<btCompoundShape> compound;
    unique_ptr<btRigidBody> chassis;
    unique_ptr<btVehicleRaycaster> ray_caster;
    unique_ptr<btRaycastVehicle> vehicle;
};

struct WorldRes {
    std::unordered_map<ObjectId, Cube> cubes;
    std::unordered_map<ObjectId, unique_ptr<Car>> cars;

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

void World::add_car(ObjectId id, glm::mat4 transform) {
    res->tasks.add([=]() {
        const double mass = 800.0;
        const double wheel_width = 0.4;
        const double wheel_radius = 0.5;
        const double connection_height = 1.0;
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
        res->cars[id] = move(car);
    });
}

void World::engine(ObjectId cid, bool run) {
    res->tasks.add([=]() {
        float force = run ? 100.0 : 0.0;
        auto it = res->cars.find(cid);
        if (it != res->cars.end()) {
            cout << "engine set to " << run << " for " << cid << endl;
            auto veh = it->second->vehicle.get();
            veh->applyEngineForce(force, 2);
            veh->applyEngineForce(force, 3);
        }
    });
}
void World::steer(ObjectId cid, float val) {
    res->tasks.add([=]() {
        auto it = res->cars.find(cid);
        if (it != res->cars.end()) {
            cout << "seer set to " << val << " for " << cid << endl;
            auto veh = it->second->vehicle.get();
            veh->setSteeringValue(val, 0);
            veh->setSteeringValue(val, 1);
        }
    });
}

void World::remove(ObjectId id) {
    res->tasks.add([=]() {
        auto cube = res->cubes.find(id);
        if (cube != res->cubes.end()) {
            res->world->removeRigidBody(cube->second.body.get());
            res->cubes.erase(cube);
        }
        auto car = res->cars.find(id);
        if (car != res->cars.end()) {
            res->world->removeRigidBody(car->second->chassis.get());
            res->cars.erase(car);
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
