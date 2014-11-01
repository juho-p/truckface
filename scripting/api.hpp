#pragma once

#include "lua.hpp"

namespace scripting {
    class Api {
        Game& game;
        Lua& l;

    public:
        Api(Game& g, Lua& lua) : game(g), l(lua) {}

        void run() {
            game.physics.run();
        }

        void stop() {
            game.physics.stop();
        }

        void add_cube() {
            ObjectId id = game.add_cube(l.num(1), l.num(2), l.num(3));
            l.ret(id);
        }

        void remove_cube() {
            game.remove_cube(l.num(1));
        }

        void register_functions() {
#define REGISTER(X) l.register_function( #X , [this]() { X(); });
            REGISTER(run)
            REGISTER(stop)
            REGISTER(add_cube)
            REGISTER(remove_cube)
#undef REGISTER
        }
    };
}
