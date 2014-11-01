#pragma once

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
    struct Camera {
        glm::vec3 pos, target, up;
    };

    class GraphicsResources;
    class Graphics : NoCopy {
        std::map<ObjectId, Cube> cubes;
        SDL_Window* window;
        void* gl_context;
        Uniforms uniforms;
        Camera camera;
        unique_ptr<GraphicsResources> res;

        void init_shaders();
        void init_cube_vao();

    public:
        Graphics();
        ~Graphics();

        void add_cube(ObjectId id, const glm::mat4& transform,
                float x, float y, float z);
        void remove(ObjectId id);
        void set_transform(ObjectId id, const glm::mat4& transform);
        void set_camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up);
        void render();
    };

    void initialize();
    void cleanup();
}
