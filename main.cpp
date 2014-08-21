#include "game.hpp"
#include "gfx/gfx.hpp"
#include "physics/world.hpp"

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    const glm::vec3 cubepos = glm::vec3(0.0f, 15.0f, -35.0f);
    const glm::vec3 cubepos2 = glm::vec3(1.1f, 17.5f, -34.0f);
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), cubepos);
    glm::mat4 trans2 = glm::translate(glm::mat4(1.0f), cubepos2);

    gfx::Graphics grap;
    physics::World phys;

    grap.add_cube(0, trans);
    grap.add_cube(1, trans2);
    phys.add_cube(0, trans, 1.0);
    phys.add_cube(1, trans2, 1.0);

    glm::mat4 groundtrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -8.0f, -35.0f));
    phys.add_cube(2, groundtrans, 0.0, 10, 1, 10);
    grap.add_cube(2, groundtrans, 10, 1, 10);

    phys.run();

    for (int i = 0; i < 300; i++) {
        auto changes = phys.get_and_reset_changes();
        cout << changes.size() << " changes" << endl;
        for (const auto& x : changes) {
            grap.set_transform(x.first, x.second);
        }
        grap.render();
    }
    cout << "stopping phys" << endl;
    phys.stop();
    cout << "stopped" << endl;
}
