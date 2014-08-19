#include "../game.hpp"

#include <vector>

class SDL_Window;

namespace gfx {
    struct Cube {
        glm::mat4 transform;
        float scale;
    };

    struct Uniforms {
        int world, world_projection, light_pos;
    };

    class GraphicsResources;
    class Graphics : NoCopy {
        std::vector<Cube> cubes;
        SDL_Window* window;
        void* gl_context;
        Uniforms uniforms;
        unique_ptr<GraphicsResources> res;

        void init_shaders();
        void init_cube_vao();

    public:
        Graphics();
        ~Graphics();

        void add_cube(const glm::mat4& transform, float scale);
        void set_transform(unsigned int index, const glm::mat4& transform);
        void render();
    };

    void initialize();
    void cleanup();
}
