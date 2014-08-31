#include "../common.hpp"

#include <map>

class SDL_Window;

namespace gfx {
    struct Cube {
        glm::mat4 transform;
        glm::vec3 scale;
    };

    struct Uniforms {
        int world, world_projection, light_pos;
    };

    class GraphicsResources;
    class Graphics : NoCopy {
        std::map<ObjectId, Cube> cubes;
        SDL_Window* window;
        void* gl_context;
        Uniforms uniforms;
        unique_ptr<GraphicsResources> res;

        void init_shaders();
        void init_cube_vao();

    public:
        Graphics();
        ~Graphics();

        void add_cube(ObjectId id, const glm::mat4& transform,
                float x=1.f, float y=1.f, float z=1.f);
        void remove(ObjectId id);
        void set_transform(ObjectId id, const glm::mat4& transform);
        void render();
    };

    void initialize();
    void cleanup();
}
