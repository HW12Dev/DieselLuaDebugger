#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


#define REAL_LUA
#include "lrdb/server.hpp"

#include <iostream>
int main() {

	lua_State* L = luaL_newstate();

	lrdb::server debug_server(21110);
	debug_server.reset(L);
	debug_server.command_stream().wait_for_connection();

	luaL_openlibs(L);

	std::cout << luaL_dofile(L, "X:/Projects/DieselLuaDebugger/test/test.lua") << std::endl;
	while (true) {
		luaL_dostring(L, "printtest()");
	}

	debug_server.reset();
	lua_close(L);

	return 0;
}