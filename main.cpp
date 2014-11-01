#include "game.hpp"
#include "common.hpp"
#include "gfx/gfx.hpp"
#include "physics/world.hpp"
#include "scripting/lua.hpp"
#include "scripting/api.hpp"


#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include <thread>
#include <atomic>

int main() {
    gfx::initialize();

    Game game;

    volatile bool keep_running = true;

    scripting::Lua lua;
    scripting::Api lua_api{game, lua};
    lua_api.register_functions();

    std::thread t([&keep_running, &lua]() {
        lua.runfile("data/scripts/repl.lua");
        keep_running = false;
    });

    while (keep_running) {
        auto changes = game.physics.get_and_reset_changes();
        for (const auto& x : changes) {
            game.graphics.set_transform(x.first, x.second);
        }
        game.graphics.render();
    }

    t.join();

    game.physics.stop();
    cout << "phys stopped" << endl;
    gfx::cleanup();
}
