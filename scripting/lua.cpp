#include "lua.hpp"

const char* LuaInit =
    "package.path = './data/scripts/?.lua'";

namespace scripting {
    Lua::Lua()  {
        L = luaL_newstate();
        luaL_openlibs(L);
        luaL_dostring(L, LuaInit);
    }

    void Lua::error(const std::string& msg) {
        std::cerr << "Lua error: " << msg << std::endl;
        throw LuaError(msg.c_str());
    }

    void Lua::getter_error(int num, const char* expected_type) {
        std::stringstream ss;
        ss << "Parameter "
            << num << " should be " << expected_type;
        std::string s = ss.str();
        error(s);
    }

    int lua_function_wrapper(lua_State* L) {
        void* funptr = lua_touserdata(L, lua_upvalueindex(1));
        void* luaptr = lua_touserdata(L, lua_upvalueindex(2));
        assert(funptr);
        assert(luaptr);
        auto pfn = static_cast<std::function<void()>*>(funptr);
        auto lua = static_cast<Lua*>(luaptr);
        lua->_return_value_count = 0;
        try {
            (*pfn)();
        } catch (LuaError e) {
        }
        return lua->_return_value_count;
    }
}
