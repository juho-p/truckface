#pragma once

#include <map>
#include "../game.hpp"

namespace physics {

    struct WorldRes;
    class World {
        WorldRes* res;
        std::map<ObjectId, glm::mat4> changes;
        void single_step_();

    public:
        World();
        ~World();

        /** Add a box to the world, can be called from other threads*/
        void add_cube(ObjectId id, glm::mat4 transform, float mass, float x=1.f, float y=1.f, float z=1.f);
        void remove(ObjectId id);

        /** Get the changes to objects, can be called from other threads */
        std::map<ObjectId, glm::mat4> get_and_reset_changes();

        /** Perform single simulation step */
        void single_step();

        /** Start running simulation in the background */
        void run();

        /** Pause background simulation, callable from other threads */
        void pause();

        /** Stop and wait for background simulation to die, callable from other threads */
        void stop();


        /** For internal logic (set change for an object) */
        void update_change(ObjectId id, const glm::mat4& change);
        /** Non thread-safe and overall retarded debug printer */
        void printworld();
    };

}
