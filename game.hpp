#pragma once

#include "common.hpp"
#include "gfx/gfx.hpp"
#include "physics/world.hpp"
#include <glm/gtc/matrix_transform.hpp>

/**
 * Some kind of integration layer for various game stuff
 *
 * This should only call thread safe functions, since it's supposed to
 * get called by some external threads
 */
struct Game {
    gfx::Graphics graphics;
    physics::World physics;
    ObjectId last_id;

    Game() : last_id(0) {
        glm::mat4 groundtrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        physics.add_cube(0, groundtrans, 0.0, 20, 1, 20);
        graphics.add_cube(0, groundtrans, 20, 1, 20);
    }

    ObjectId add_cube(float x, float y, float z, float size) {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        auto id = new_id();
        physics.add_cube(id, trans, size*size*size, size, size, size);
        graphics.add_cube(id, trans, size, size, size);
        return id;
    }

    ObjectId add_car(float x, float y, float z) {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        auto id = new_id();
        physics.add_car(id, trans);
        graphics.add_cube(id, trans, 1.0f, 0.5f, 2.0f);
        return id;
    }

    void remove_cube(ObjectId id) {
        graphics.remove(id);
        physics.remove(id);
    }

    void sync_changes() {
        auto changes = physics.get_and_reset_changes();
        for (const auto& x : changes) {
            graphics.set_transform(x.first, x.second);
        }
    }

    ObjectId new_id() { return ++last_id; }
};
