#pragma once

#include <map>
#include "../game.hpp"

namespace physics {

    struct WorldRes;
    class World {
        WorldRes* res;
        std::map<ObjectId, glm::mat4> changes;

    public:
        World();
        ~World();

        /** Add a box to the world */
        void add_cube(ObjectId id, glm::mat4 transform, float mass, float x=1.f, float y=1.f, float z=1.f);

        /** Get the changes to objects */
        std::map<ObjectId, glm::mat4> get_and_reset_changes();

        /** For internal logic (set change for an object) */
        void update_change(ObjectId id, const glm::mat4& change);

        /** Perform single simulation step */
        void single_step();

        /** Start running simulation in the background */
        void run();

        /** Pause background simulation */
        void pause();

        /** Stop and wait for background simulation to finish */
        void stop();

        void printworld();
    };

}
