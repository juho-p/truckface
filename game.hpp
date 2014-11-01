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
        glm::mat4 groundtrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -8.0f, -35.0f));
        physics.add_cube(0, groundtrans, 0.0, 10, 1, 10);
        graphics.add_cube(0, groundtrans, 10, 1, 10);
    }

    ObjectId add_cube(float x, float y, float z) {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        auto id = new_id();
        physics.add_cube(id, trans, 1.0);
        graphics.add_cube(id, trans);
        return id;
    }

    void remove_cube(ObjectId id) {
        graphics.remove(id);
        physics.remove(id);
    }

    ObjectId new_id() { return ++last_id; }
};
