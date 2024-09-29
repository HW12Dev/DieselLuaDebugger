#pragma once

#include "luajit_defines.h"

#include <cstddef>
#include <stdint.h>

#define CONCAT_IMPL(...) __VA_ARGS__
#define CONCAT_ARGS(...) CONCAT_IMPL(__VA_ARGS__)

#ifndef DIESELLUAIMPL
#define LUA_FUNCTION_(name, return_, pattern, mask, function_args) typedef return_ (__cdecl*name##_type)(function_args); extern name##_type name;
#else
#define LUA_FUNCTION_(name, return_, pattern, mask, function_args) typedef return_ (__cdecl*name##_type)(function_args); name##_type name; ScanForSig name##_sigsearch (#name, pattern, mask, (unsigned long long*)&name);
#endif





#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))

#define LUA_NUMBER_DOUBLE
#define LUA_NUMBER		double
#define LUAI_UACNUMBER		double
#define LUA_NUMBER_SCAN		"%lf"
#define LUA_NUMBER_FMT		"%.14g"
#define lua_number2str(s, n)	sprintf((s), LUA_NUMBER_FMT, (n))
#define LUAI_MAXNUMBER2STR	32
#define LUA_INTFRMLEN		"l"
#define LUA_INTFRM_T		long
#define LUA_INTEGER	std::ptrdiff_t
#define LUA_IDSIZE	60	/* Size of lua_Debug.short_src. */


#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_MULTRET	(-1)

/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;


/* type for integer functions */
typedef LUA_INTEGER lua_Integer;

typedef union TValue;

struct lua_State;

struct lua_Debug {
  int event;
  const char* name;	/* (n) */
  const char* namewhat;	/* (n) `global', `local', `field', `method' */
  const char* what;	/* (S) `Lua', `C', `main', `tail' */
  const char* source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

typedef void (*lua_Hook) (lua_State* L, lua_Debug* ar);
typedef void* (*lua_Alloc) (void* ud, void* ptr, size_t osize, size_t nsize);
typedef const char* (*lua_Reader) (lua_State* L, void* ud, size_t* sz);
typedef int (*lua_Writer) (lua_State* L, const void* p, size_t sz, void* ud);
typedef int (*lua_CFunction) (lua_State* L);

/// Debug API

/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4

typedef uint32_t MSize;
#if LJ_GC64
typedef uint64_t GCSize;
#else
typedef uint32_t GCSize;
#endif
/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

#define RAIDWW2

#if defined(RAIDWW2) && defined(BEFORE_RAID_15703546)
extern int __cdecl lua_getinfo(lua_State* L, const char* what, lua_Debug* ar); // RAID WW2 function is too short for it's normal signature
extern const void* lua_topointer(lua_State* L, int idx);
extern void lua_getfenv(lua_State* L, int idx);
extern int lua_setfenv(lua_State* L, int idx);
//extern const char* lua_setupvalue(lua_State* L, int funcindex, int n);
LUA_FUNCTION_(lua_setupvalue, const char*, "\x48\x8B\xC4\x48\x89\x58\x00\x48\x89\x68\x00\x48\x89\x70\x00\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x90\x00\x00\x00\x0F\x29\x70\x00\x0F\x29\x78\x00\x48\x8B\x05", "xxxxxx?xxx?xxx?xxxxxxxxxxxxxxxxxxx?xxx?xxx", CONCAT_ARGS(lua_State* L, int funcindex, int n));
//LUA_FUNCTION_(lua_getinfo, int, "", "", CONCAT_ARGS(lua_State* L, const char* what, lua_Debug* ar));
//LUA_FUNCTION_(lua_topointer, const void*, "", "", CONCAT_ARGS(lua_State* L, int idx));
//LUA_FUNCTION_(lua_getfenv, void, "", "", CONCAT_ARGS(lua_State* L, int idx));
//LUA_FUNCTION_(lua_setfenv, int, "", "", CONCAT_ARGS(lua_State* L, int idx));

// some sigs from https://gitlab.com/cpone/diesel-superblt/-/blob/all_games-raid_u21_beta/platforms/w32/game/raid/signatures/sigdef_game.h?ref_type=heads

// basic stack manipulation
LUA_FUNCTION_(lua_gettop, int, "\x48\x8B\x41\x28\x48\x2B\x41\x20", "xxxxxxxx", CONCAT_ARGS(lua_State* L));
LUA_FUNCTION_(lua_settop, void, "\x40\x53\x48\x83\xEC\x20\x48\x8B\xD9\x85\xD2\x78\x7A", "xxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_pushvalue, void, "\x48\x83\xEC\x28\x4C\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x49\x8B\x53\x28\x48\x8B\x00", "xxxxxxxx????xxxxxxx", CONCAT_ARGS(lua_State* L, int idx));


// access functions (stack -> C)
LUA_FUNCTION_(lua_type, int, "\x48\x83\xEC\x28\x4C\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x48\x8B\xD0", "xxxxxxxx????xxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_typename, const char*, "\x8D\x42\x01\x48\x98\x48\x8D\x0D", "xxxxxxxx", CONCAT_ARGS(lua_State* L, int tp));

LUA_FUNCTION_(lua_tonumber, lua_Number, "\x48\x83\xEC\x28\xE8\xCC\xCC\xCC\xCC\x48\x8B\x10\x48\x8B\xCA\x48\xC1\xF9\x2F\x83\xF9\xF2\x77\x09", "xxxxx????xxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_toboolean, int, "\x48\x83\xEC\x28\xE8\xCC\xCC\xCC\xCC\x48\x8B\x08\x33\xC0", "xxxxx????xxxxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_tolstring, const char*, "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x49\x8B\xF8\x8B\xDA\x48\x8B\xF1", "xxxx?xxxx?xxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int idx, size_t* len));
LUA_FUNCTION_(lua_objlen, size_t, "\x40\x53\x48\x83\xEC\x20\x4C\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x48\x8B\xD8", "xxxxxxxxxx????xxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_touserdata, void*, "\x48\x83\xEC\x28\xE8\xCC\xCC\xCC\xCC\x48\x8B\x00", "xxxxx????xxx", CONCAT_ARGS(lua_State* L, int idx));



// push functions (C -> stack)
LUA_FUNCTION_(lua_pushnil, void, "\x48\x8B\x41\x28\x48\xC7\x00\xFF\xFF\xFF\xFF\x48\x83\x41\x28\x08\x48\x8B\x41\x28", "xxxxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L));
LUA_FUNCTION_(lua_pushnumber, void, "\x48\x8B\x41\x28\xF2\x0F\x11\x08", "xxxxxxxx", CONCAT_ARGS(lua_State* L, lua_Number n));
LUA_FUNCTION_(lua_pushlstring, void, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x4C\x8B\x49\x10\x49\x8B\xF8", "xxxxxxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, const char* s, size_t l));
LUA_FUNCTION_(lua_pushboolean, void, "\x48\x83\xEC\x28\x48\xB8\x00\x00\x00\x00\x00\x80\x00\x00", "xxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int b));
LUA_FUNCTION_(lua_pushlightuserdata, void, "\x48\x83\xEC\x28\x48\xB8\x00\x00\x00\x00\x00\x80\x00\x00", "xxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, void* p));


// get functions (lua -> stack)
LUA_FUNCTION_(lua_rawget, void, "\x40\x53\x48\x83\xEC\x20\x4C\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x49\x8B\x5B\x28", "xxxxxxxxxx????xxxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_createtable, void, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x4C\x8B\x49\x10\x41\x8B\xF8", "xxxxxxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int narr, int nrec));

// set functions (stack -> Lua)
LUA_FUNCTION_(lua_setfield, void, "\x48\x89\x5C\x24\x08\x57\x48\x83\xEC\x20\x4D\x8B\xD8\x48\x8B\xD9", "xxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int idx, const char* k));
LUA_FUNCTION_(lua_rawset, void, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x48\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x48\x8B\x73\x28", "xxxxxxxxxxxxxxxxxxx????xxxx", CONCAT_ARGS(lua_State* L, int idx));
LUA_FUNCTION_(lua_rawseti, void, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x4D\x63\xD8", "xxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int idx, int n));
LUA_FUNCTION_(lua_setmetatable, int, "\x48\x83\xEC\x28\x4C\x8B\xD9\xE8\xCC\xCC\xCC\xCC\x49\x8B\x53\x28\x4C\x8B\xC8", "xxxxxxxx????xxxxxxx", CONCAT_ARGS(lua_State* L, int objindex));

// `load' and `call' functions (load and run Lua code)
LUA_FUNCTION_(lua_pcall, int, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x48\x8B\x59\x10\x41\x8B\xF0", "xxxxxxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int nargs, int nresults, int errfunc));

// miscellaneous functions
LUA_FUNCTION_(lua_next, int, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xD9\xE8\x00\x00\x00\x00\x4C\x8B\x43", "xxxx?xxxxxxxxx????xxx", CONCAT_ARGS(lua_State* L, int idx));

// lauxlib.h
LUA_FUNCTION_(luaL_callmeta, int, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x8B\xFA\x48\x8B\xD9\xE8\x00\x00\x00\x00\x85\xC0", "xxxx?xxxxxxxxxxx????xx", CONCAT_ARGS(lua_State* L, int obj, const char* e));

LUA_FUNCTION_(luaL_loadstring, int, "\x48\x83\xEC\x48\x48\x89\x54\x24\x30", "xxxxxxxxx", CONCAT_ARGS(lua_State* L, const char* s));

LUA_FUNCTION_(lua_loadx, int, "\x4C\x8B\xDC\x53\x56\x57\x48\x81\xEC\xE0\x00\x00\x00", "xxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, lua_Reader reader, void* data, const char* chunkname, const char* mode));


LUA_FUNCTION_(index2adr, TValue*, "\x4C\x8B\xC1\x85\xD2\x7E\x00\x8D\x42\x00\x48\x63\xD0\x48\x8B\x41\x00\x48\x8D\x04\xD0", "xxxxxx?xx?xxxxxx?xxxx", CONCAT_ARGS(lua_State* L, int idx));

LUA_FUNCTION_(luaL_getmetafield, int, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x49\x8B\xD8\x48\x8B\xF9\xE8\x00\x00\x00\x00\x85\xC0", "xxxx?xxxxxxxxxxxx????xx", CONCAT_ARGS(lua_State* L, int obj, const char* e));

LUA_FUNCTION_(lua_getfield, void, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x4D\x8B\xD8\x48\x8B\xD9\xE8\x00\x00\x00\x00\x48\x8B\xF8\x49\x83\xC8\xFF\x0F\x1F\x40\x00\x49\xFF\xC0\x43\x80\x3C\x03\x00\x75\x00\x49\x8B\xD3\x48\x8B\xCB\xE8\x00\x00\x00\x00\x48\xB9\x00\x00\x00\x00\x00\x80\xFD\xFF\x4C\x8D\x44\x24\x00\x48\x0B\xC1\x48\x8B\xD7\x48\x8B\xCB\x48\x89\x44\x24\x00\xE8\x00\x00\x00\x00\x48\x85\xC0\x75", "xxxx?xxxxxxxxxxxx????xxxxxxxxxx?xxxxxxxxx?xxxxxxx????xxxxxxxxxxxxxx?xxxxxxxxxxxxx?x????xxxx", CONCAT_ARGS(lua_State* L, int idx, const char* k));


// Functions to be called by the debuger in specific events
LUA_FUNCTION_(lua_getstack, int, "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x49\x8B\xF8\x8B\xDA\x4C\x8D\x44\x24", "xxxx?xxxx?xxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int level, lua_Debug* ar));
LUA_FUNCTION_(lua_getlocal, const char*, "\x40\x53\x48\x83\xEC\x20\x48\xC7\x44\x24\x00\x00\x00\x00\x00\x48\x8B\xD9", "xxxxxxxxxx?????xxx", CONCAT_ARGS(lua_State* L, const lua_Debug* ar, int n));
LUA_FUNCTION_(lua_setlocal, const char*, "\x40\x53\x48\x83\xEC\x20\x45\x8B\xC8", "xxxxxxxxx", CONCAT_ARGS(lua_State* L, const lua_Debug* ar, int n));
LUA_FUNCTION_(lua_getupvalue, const char*, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xF9\x45\x8D\x58", "xxxx?xxxxxxxxxxx", CONCAT_ARGS(lua_State* L, int funcindex, int n));
LUA_FUNCTION_(lua_sethook, int, "\x48\x83\xEC\x28\x48\x8B\x49\x00\x41\x8B\xC0", "xxxxxxx?xxx", CONCAT_ARGS(lua_State* L, lua_Hook func, int mask, int count));


LUA_FUNCTION_(lua_pushcclosure, void, "\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x4C\x8B\x49\x10\x48\x8B\xF2", "xxxxxxxxxxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, lua_CFunction fn, int n))

LUA_FUNCTION_(lua_pushstring, void, "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\xFA\x48\x8B\xD9\x48\x85\xD2", "xxxx?xxxxxxxxxxxxxx", CONCAT_ARGS(lua_State* L, const char* s));

LUA_FUNCTION_(lj_state_growstack, void, "\x40\x53\x48\x83\xEC\x20\x8B\x41\x00\x48\x8B\xD9\x3D\xE4\xFF\x00\x00\x77\x00\x03\xD0", "xxxxxxxx?xxxxxxxxx?xx", CONCAT_ARGS(lua_State* L, MSize need));

#elif defined(RAIDWW2)

#include "generated_lua_definitions.txt"

#endif

#define lua_pop(L,n)		lua_settop(L, -(n)-1)

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)

#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_getglobal(L,s)	lua_getfield(L, LUA_GLOBALSINDEX, (s))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)
#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))


#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)