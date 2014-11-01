#include "api.hpp"

// YES, C preprocessor is the greatest!

#define defun(name) l.register_function(#name, [&] {
#define endfun });
#define simplefun(name, line) l.register_function(#name, [&] { line ; } );

void scripting::register_game_functions(Game& game, Lua& l) {

    simplefun(run, game.physics.run());
    simplefun(stop, game.physics.stop());

    defun(add_cube)
        ObjectId id = game.add_cube(l.num(1), l.num(2), l.num(3), l.argc() > 3 ? l.num(4) : 0.5);
        l.ret(id);
    endfun
    defun(remove_cube)
       game.remove_cube(l.num(1));
    endfun 

    defun(setcam)
        game.graphics.set_camera(
                glm::vec3(l.num(1), l.num(2), l.num(3)),
                glm::vec3(l.num(4), l.num(5), l.num(6)),
                glm::vec3(0.f, 1.f, 0.f));
    endfun
}
