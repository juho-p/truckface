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
    scripting::register_game_functions(game, lua);

    std::thread t([&keep_running, &lua]() {
        lua.runfile("data/scripts/repl.lua");
        keep_running = false;
    });

    while (keep_running) {
        game.sync_changes();
        game.graphics.render();
    }

    t.join();

    game.physics.stop();
    cout << "phys stopped" << endl;
    gfx::cleanup();
}
