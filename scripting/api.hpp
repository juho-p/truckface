#pragma once

#include "lua.hpp"
#include "../game.hpp"

namespace scripting {
    void register_game_functions(Game& game, Lua& lua);
}
