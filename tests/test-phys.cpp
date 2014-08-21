#include "../game.hpp"
#include "../physics/world.hpp"
#include <glm/gtc/matrix_transform.hpp>

int main() {
    const glm::vec3 cubepos = glm::vec3(0.0f, 15.0f, -35.0f);
    const glm::vec3 cubepos2 = glm::vec3(1.1f, 17.5f, -34.0f);
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), cubepos);
    glm::mat4 trans2 = glm::translate(glm::mat4(1.0f), cubepos2);

    physics::World phys;

    phys.add_cube(0, trans, 1.0);
    phys.add_cube(1, trans2, 1.0);

    glm::mat4 groundtrans = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, -30.0f)), glm::vec3(50,1,50));
    phys.add_cube(2, groundtrans, 0.0);

    for (int i = 0; i < 5; i++) {
        phys.single_step();
    }
    phys.printworld();
}
