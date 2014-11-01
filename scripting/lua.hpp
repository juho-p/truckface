#pragma once

#include "../common.hpp"
#include <sstream>
#include <forward_list>
#include <functional>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

namespace scripting {

    struct LuaError : public std::runtime_error {
        LuaError(const char* m) : std::runtime_error(m) {}
    };

    int lua_function_wrapper(lua_State* L);

    /**
     * Thin Lua wrapper
     */
    class Lua {
        lua_State* L;
        using string = std::string;
        std::forward_list<std::function<void()>> stored_functions;

    public:
        int _return_value_count;

        Lua() {
            L = luaL_newstate();
            luaL_openlibs(L);
        }
        ~Lua() {
            lua_close(L);
        }

        void error(const std::string& msg) {
            std::cerr << "Lua error: " << msg << std::endl;
            throw LuaError(msg.c_str());
        }

        void getter_error(int num, const char* expected_type) {
            std::stringstream ss;
            ss << "Parameter "
                << num << " should be " << expected_type;
            std::string s = ss.str();
            error(s);
        }

        void push(double num) {
            lua_pushnumber(L, num);
        }
        void push(const char* str) {
            lua_pushstring(L, str);
        }
        void push(const string& str) {
            push(str.c_str());
        }
        double num(int i) {
            if (lua_isnumber(L, i)) return lua_tonumber(L, i);
            getter_error(i, "number");
            return 0;
        }
        string str(int i) {
            if (lua_isstring(L, i)) return lua_tostring(L, i);
            getter_error(i, "string");
            return "";
        }
        int argc() {
            return lua_gettop(L);
        }

        template <typename Fn>
        void register_function(const char* name, Fn fn) {
            stored_functions.push_front(fn);
            lua_pushlightuserdata(L, &stored_functions.front());
            lua_pushlightuserdata(L, this);

            lua_pushcclosure(L, lua_function_wrapper, 2);

            lua_setglobal(L, name);
        }

        void ret() {}
        template<typename Arg0, typename ... Args>
        void ret(Arg0 arg, Args... args) {
            push(arg);
            _return_value_count++;
            ret(args...);
        }

        void runfile(const char* file) {
            luaL_dofile(L, file);
        }
    };
}
