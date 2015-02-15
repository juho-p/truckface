#include "api.hpp"

#ifdef USE_READLINE

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

const char* Prompt = "> ";

inline string read_console_line(const char* prompt) {
    char* buf = readline(prompt);
    string s{buf};
    if (s.size() > 0) add_history(buf);
    free(buf);
    return s;
}

#else

inline string read_console_line(const char* prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

#endif

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
    defun(add_car)
        ObjectId id = game.add_car(l.num(1), l.num(2), l.num(3));
        l.ret(id);
    endfun
    defun(remove_cube)
       game.remove_cube(l.num(1));
    endfun 

    defun(carengine)
        game.physics.engine(l.num(1), l.num(2));
    endfun
    defun(carsteer)
        game.physics.steer(l.num(1), l.num(2));
    endfun

    defun(setcam)
        game.graphics.set_camera(
                glm::vec3(l.num(1), l.num(2), l.num(3)),
                glm::vec3(l.num(4), l.num(5), l.num(6)),
                glm::vec3(0.f, 1.f, 0.f));
    endfun

    defun(readline)
        auto prompt = l.str(1);
        auto line = read_console_line(prompt.c_str());
        l.ret(line);
    endfun
    defun(writeline)
        cout << l.str(1) << endl;
    endfun
}
