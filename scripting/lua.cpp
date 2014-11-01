#include "lua.hpp"

namespace scripting {
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
