#pragma once

#include <functional>
#include "../game.hpp"

namespace physics {

    struct WorldRes;
    class World {
        WorldRes* res;

    public:
        World();
        ~World();

        void add_cube(glm::vec3 pos, float scale,
                std::function<void(glm::mat4)> transform_changed);

        void step();

        void printworld();
    };

}
