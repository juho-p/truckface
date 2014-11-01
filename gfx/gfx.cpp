#include "gfx.hpp"
#include "../util/file.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <SDL2/SDL.h>

#include <sstream>
#include <vector>
#include <cstddef>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../util/task_list.hpp"

const static int POSITION = 1,
      NORMAL = 2;

struct V3 {
    float x, y, z;
};
struct Vertex {
    V3 pos;
    V3 normal;
};

#define LEN(A) (sizeof(A) / sizeof(*A))

static Vertex VERTICES[] = {
    Vertex { V3 { 0.9, 0.9, 1.0}, V3 { 0.2, 0.2, 1.0} },
    Vertex { V3 {-0.9, 0.9, 1.0}, V3 {-0.2, 0.2, 1.0} },
    Vertex { V3 {-0.9,-0.9, 1.0}, V3 {-0.2,-0.2, 1.0} },
    Vertex { V3 { 0.9,-0.9, 1.0}, V3 { 0.2,-0.2, 1.0} },

    Vertex { V3 { 0.9, 0.9,-1.0}, V3 { 0.2, 0.2,-1.0} },
    Vertex { V3 {-0.9, 0.9,-1.0}, V3 {-0.2, 0.2,-1.0} },
    Vertex { V3 {-0.9,-0.9,-1.0}, V3 {-0.2,-0.2,-1.0} },
    Vertex { V3 { 0.9,-0.9,-1.0}, V3 { 0.2,-0.2,-1.0} },

    Vertex { V3 {-1.0, 0.9, 0.9}, V3 {-1.0, 0.2, 0.2} },
    Vertex { V3 {-1.0, 0.9,-0.9}, V3 {-1.0, 0.2,-0.2} },
    Vertex { V3 {-1.0,-0.9,-0.9}, V3 {-1.0,-0.2,-0.2} },
    Vertex { V3 {-1.0,-0.9, 0.9}, V3 {-1.0,-0.2, 0.2} },

    Vertex { V3 { 1.0, 0.9,-0.9}, V3 { 1.0, 0.2,-0.2} },
    Vertex { V3 { 1.0, 0.9, 0.9}, V3 { 1.0, 0.2, 0.2} },
    Vertex { V3 { 1.0,-0.9, 0.9}, V3 { 1.0,-0.2, 0.2} },
    Vertex { V3 { 1.0,-0.9,-0.9}, V3 { 1.0,-0.2,-0.2} },

    Vertex { V3 { 0.9, 1.0,-0.9}, V3 { 0.2, 1.0,-0.2} },
    Vertex { V3 {-0.9, 1.0,-0.9}, V3 {-0.2, 1.0,-0.2} },
    Vertex { V3 {-0.9, 1.0, 0.9}, V3 {-0.2, 1.0, 0.2} },
    Vertex { V3 { 0.9, 1.0, 0.9}, V3 { 0.2, 1.0, 0.2} },

    Vertex { V3 { 0.9,-1.0,-0.9}, V3 { 0.2,-1.0,-0.2} },
    Vertex { V3 { 0.9,-1.0, 0.9}, V3 { 0.2,-1.0, 0.2} },
    Vertex { V3 {-0.9,-1.0, 0.9}, V3 {-0.2,-1.0, 0.2} },
    Vertex { V3 {-0.9,-1.0,-0.9}, V3 {-0.2,-1.0,-0.2} }
};

static GLint INDICES[] = {
    0, 1, 2, 0, 2, 3,
    7, 6, 4, 6, 5, 4,
    8, 9, 10, 8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,

    //top edges
    0, 19, 18, 0, 18, 1,
    4, 17, 16, 4, 5, 17,
    8, 18, 17, 8, 17, 9,
    13, 19, 16, 13, 16, 12,
    //bottom edges 20, 21, 22, 23
    2, 22, 21, 2, 21, 3,
    6, 7, 20, 6, 20, 23,
    22, 11, 10, 22, 10, 23,
    21, 15, 20, 21, 14, 15,
    //lefty
    2, 1, 8, 2, 8, 11,
    5, 6, 10, 5, 10, 9,
    //righty
    0, 3, 14, 0, 14, 13,
    4, 12, 15, 4, 15, 7,

    //corners
    0, 13, 19,
    1, 18, 8,
    2, 11, 22,
    3, 21, 14,
    4, 16, 12, 
    5, 9, 17,
    6, 23, 10,
    7, 15, 20
};

static void throw_sdl_error(const char* msg) {
    std::stringstream ss;
    ss << msg << ": " << SDL_GetError();
    throw std::runtime_error(ss.str());
}

static void check_sdl_error(const char* msg) {
    const char* err = SDL_GetError();
    if (err && *err) {
        SDL_ClearError();
        cerr << msg << ": " << err << endl;
    }
}

static void check_gl_error(const char* msg) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        cerr << msg << ": GL error code " << std::hex << err << endl;
    }
}

static void init_gl() {
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
}

inline glm::mat4 projection_matrix(const gfx::Camera& camera) {
    auto proj = glm::perspective(45.0f, 1.0f, 0.1f, 100.f);
    auto cam = glm::lookAt(camera.pos, camera.target, camera.up);
    return proj * cam;
}

namespace gfx {

#include "vertex_array.hpp"
#include "shader_program.hpp"

struct GraphicsResources {
    ShaderProgram program;
    VertexArray cube_vao;
    util::TaskList tasks;
};

void Graphics::init_cube_vao() {
    VertexArray& vao = res->cube_vao;

    vao.set_vertex_buffer(VERTICES, VERTICES+LEN(VERTICES), GL_STATIC_DRAW);
    vao.set_index_buffer(INDICES, INDICES+LEN(INDICES), GL_STATIC_DRAW);
    vao.set_pointer(POSITION, 3, offsetof(Vertex, pos));
    vao.set_pointer(NORMAL, 3, offsetof(Vertex, normal));
}

void Graphics::init_shaders() {
    ShaderProgram& prog = res->program;

    auto vs = util::read_file("data/shaders/render_vertex.glsl");
    auto fs = util::read_file("data/shaders/render_fragment.glsl");

    prog.add_shader_from_source(GL_VERTEX_SHADER, &vs[0]);
    prog.add_shader_from_source(GL_FRAGMENT_SHADER, &fs[0]);

    prog.bind_attribute_location("position", POSITION);
    prog.bind_attribute_location("normal", NORMAL);
    prog.link();
}

void initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        throw_sdl_error("SDL_Init failed");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    check_sdl_error("after gfx initialize");
}

void cleanup() {
    SDL_Quit();
}

Graphics::Graphics() {
    this->window = SDL_CreateWindow(
            "test", 0, 0, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!this->window) {
        throw_sdl_error("Failed to create window");
    }
    auto ctx = SDL_GL_CreateContext(this->window);
    if (!ctx) {
        throw_sdl_error("Failed to create GL context");
    }
    SDL_GL_MakeCurrent(this->window, ctx);
    init_gl();
    SDL_GL_SetSwapInterval(1);
    check_sdl_error("After Graphics construct");
    this->gl_context = ctx;

    this->res = unique_ptr<GraphicsResources>(new GraphicsResources);

    this->init_shaders();
    this->init_cube_vao();
    this->uniforms.world = res->program.get_uniform_location("world");
    this->uniforms.world_projection = res->program.get_uniform_location("world_projection");
    this->uniforms.light_pos = res->program.get_uniform_location("light_pos");

    camera.pos = glm::vec3(0, 5, 35);
    camera.target = glm::vec3(0, 0, 0);
    camera.up = glm::vec3(0, 1, 0);
}

Graphics::~Graphics() {
    SDL_GL_DeleteContext(this->gl_context);
    SDL_DestroyWindow(this->window);
}

void Graphics::add_cube(ObjectId id, const glm::mat4& transform,
        float x, float y, float z) {
    res->tasks.add([=]() {
        this->cubes.emplace(std::make_pair(id, Cube{ transform, glm::vec3(x, y, z) }));
    });
}
void Graphics::remove(ObjectId id) {
    res->tasks.add([=]() {
        this->cubes.erase(id);
    });
}

void Graphics::render() {
    res->tasks.run();
    check_gl_error("before render");
    res->program.activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform3f(this->uniforms.light_pos, 20.0f, 20.0f, 20.0f);
    auto projection = projection_matrix(camera);
    for (const auto& kv : this->cubes) {
        const Cube& c = kv.second;

        auto world_proj = projection * glm::scale(c.transform, c.scale);

        glUniformMatrix4fv(uniforms.world, 1, GL_FALSE, glm::value_ptr(c.transform));
        glUniformMatrix4fv(uniforms.world_projection, 1, GL_FALSE, glm::value_ptr(world_proj));

        res->cube_vao.draw();
    }
    SDL_GL_SwapWindow(this->window);
    check_gl_error("after render");
}

void Graphics::set_transform(ObjectId index, const glm::mat4& transform) {
    auto cube = this->cubes.find(index);
    if (cube != this->cubes.end()) {
        cube->second.transform = transform;
    }
}

void Graphics::set_camera(glm::vec3 pos, glm::vec3 target, glm::vec3 up) {
    res->tasks.add([=]() {
        this->camera.pos = pos;
        this->camera.target = target;
        this->camera.up = up;
    });
}

}; // end namespace gfx
