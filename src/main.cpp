#include "generated_game_information.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "diesel.hpp"
#include "lrdb/server.hpp"
#include "lrdb/message.hpp"


#include <psapi.h>

#include <MinHook.h>

#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>


#include <fstream>
#include <filesystem>
#include <iostream>
#include <luajit_types.h>

#include "lua_decompiler.h"

extern void scan_for_all_lua_functions(const char* executable);
unsigned __int64 FindPattern(std::string module, const char* pattern, const char* mask);
extern std::map<dsl::idstring, std::string> hashlist;

bool g_wait_for_debugger = false;
extern bool g_force_new_decompilation;
bool g_never_update_lua = false;

class LRDBWrapper {
public:
  LRDBWrapper(bool wait_for_connect, uint16_t port) {
    this->lrdb = new lrdb::server(wait_for_connect, port);
    this->wait_for_debug = wait_for_connect;
  }
  ~LRDBWrapper() {
    delete lrdb;
  }

public:
  lrdb::server* lrdb;
  bool wait_for_debug;
};

dsl::idstring lua___idstring = dsl::idstring("lua");

//const char* dsl__LuaInterface__newstate_pattern = "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x41\x0F\xB6\xF8\x0F\xB6\xF2";
//const char* dsl__LuaInterface__newstate_mask = "xxxx?xxxx?xxxxxxxxxxxx";

//const char* lua_close_pattern = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\x8B\x79\x00\x48\x8B\x9F";
//const char* lua_close_mask = "xxxx?xxxx?xxxx?xxxxxxxx?xxx";

const char* dsl__LuaInterpreter__ctor_pattern = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x4C\x24\x00\x57\x48\x83\xEC\x40";
const char* dsl__LuaInterpreter__ctor_mask = "xxxx?xxxx?xxxx?xxxx?xxxxx";

const char* dsl__LuaInterface__dtor_pattern = "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20\x33\xDB\x48\x8B\xF9\x48\x39\x0D\x00\x00\x00\x00\x75\x00\x48\x89\x1D\x00\x00\x00\x00\x88\x59\x00\x48\x8B\x09\xE8\x00\x00\x00\x00\x4C\x8B\x0D\x00\x00\x00\x00\x48\xB8\xAB\xAA\xAA\xAA\xAA\xAA\xAA\xAA\x4C\x8B\x05\x00\x00\x00\x00\x49\x8B\xD1\x49\x2B\xD0\x48\xC1\xFA\x03\x48\x0F\xAF\xD0\x48\x85\xD2\x74\x00\x48\x8B\xC3\x0F\x1F\x84\x00\x00\x00\x00\x00\x48\x8D\x04\x40\x49\x39\x3C\xC0\x49\x8D\x0C\xC0\x74\x00\xFF\xC3\x8B\xC3\x48\x3B\xC2\x72\x00\xEB\x00\x48\x8D\x51\x00\x4C\x2B\xCA\x4D\x8B\xC1\xE8\x00\x00\x00\x00\x48\x83\x2D\x00\x00\x00\x00\x00\x48\x8D\x4F\x00\xE8\x00\x00\x00\x00\x48\x8D\x4F\x00\xFF\x15\x00\x00\x00\x00\x48\x8D\x4F\x00\xE8";
const char* dsl__LuaInterface__dtor_mask = "xxxx?xxxxxxxxxxxxx????x?xxx????xx?xxxx????xxx????xxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxx?xxxxxxx????xxxxxxxxxxxxx?xxxxxxxx?x?xxx?xxxxxxx????xxx?????xxx?x????xxx?xx????xxx?x";
const char* lua_load_pattern = "\x4C\x8B\xDC\x49\x89\x5B\x00\x57\x48\x81\xEC\xE0\x00\x00\x00\x4D\x85\xC9\x49\x89\x53\x00\x4D\x89\x43\x00\x48\x8D\x05\x00\x00\x00\x00\x49\x0F\x45\xC1\x49\x89\x4B\x00\x49\x89\x43\x00\x4C\x8D\x0D\x00\x00\x00\x00\x33\xC0";
const char* lua_load_mask = "xxxxxx?xxxxxxxxxxxxxx?xxx?xxx????xxxxxxx?xxx?xxx????xx";

const char* raid_string_assign_pattern = "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x20\x48\xBF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F";
const char* raid_string_assign_mask = "xxxx?xxxx?xxxx?xxxxxxxxxxxxxxx";

//std::string::assign, in the raid executable
typedef void* (__fastcall* raid_string_assign_t)(__int64* this_, const void* a2, size_t _Count);
raid_string_assign_t raid_string_assign;

// dsl::idstring::t, returns a string representation of an idstring
const char* dsl__idstring__t_pattern = "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x70\x48\x8B\xDA\x33\xFF\x48\x8B\xD1";
const char* dsl__idstring__t_mask = "xxxx?xxxxxxxxxxxxx";
// dsl::idstring::t, returns a string representation of an idstring
typedef std::string* (__fastcall* dsl__idstring__t_t)(dsl::idstring* this_, std::string* result);
dsl__idstring__t_t dsl__idstring__t;
dsl__idstring__t_t dsl__idstring__t_o;




//struct lua_State;
//typedef const char* (*lua_Reader) (lua_State* L,
//  void* data,
//  size_t* size);
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5

typedef __int64(__fastcall* lua_load_type)(lua_State* L, lua_Reader reader, dsl::Buffer* data, const char* chunkname);
//typedef __int64(__fastcall* luaL_load_type)(lua_State* L, lua_Reader reader, void* data, const char* chunkname);

lua_load_type lua_load;
lua_load_type lua_load_o;

namespace dsl {
  class LuaInterface {
  public:
    lua_State* L;
  };
  class LuaInterpreter : public dsl::LuaInterface{

  };
}

typedef __int64(__fastcall* dsl__LuaInterface__dtor_type)(dsl::LuaInterface* this_);
dsl__LuaInterface__dtor_type dsl__LuaInterface__dtor;
dsl__LuaInterface__dtor_type dsl__LuaInterface__dtor_o;
//typedef __int64(__fastcall* dsl__LuaInterface__newstate_type)(dsl::LuaInterface* this_, bool libs, bool main, int allocation);
//dsl__LuaInterface__newstate_type dsl__LuaInterface__newstate;
//dsl__LuaInterface__newstate_type dsl__LuaInterface__newstate_o;

typedef dsl::LuaInterpreter* (__fastcall* dsl__LuaInterpreter__ctor_type)(dsl::LuaInterpreter* this_, __int64 thread, bool libs, bool classes, bool main, int allocation);
dsl__LuaInterpreter__ctor_type dsl__LuaInterpreter__ctor;
dsl__LuaInterpreter__ctor_type dsl__LuaInterpreter__ctor_o;


#if defined(RAIDWW2)
#define executable "raid_win64_release.exe"
#elif defined(PD2)
#define executable "payday2_win32_release.exe"
#elif defined(PDTH)
#define executable "payday_win32_release.exe"
#endif


// the result string is created on the stack by dsl::ScriptIdstring::to_string
std::string* __fastcall dsl__idstring__t_h(dsl::idstring* this_, std::string* result) {
  if (!hashlist.contains(*this_)) {
    dsl__idstring__t_o(this_, result);
  } else
  {
    auto& hashlist_entry = hashlist[*this_];
    raid_string_assign((__int64*)result, hashlist_entry.c_str(), hashlist_entry.size());
  }
  return result;
}

bool diesel_lua_debugger_breakpoint_path_handler(lrdb::debugger& debugger, const std::string& breakpoint_path_, const std::string& executing_path_) {
  auto breakpoint_path = breakpoint_path_;
  auto executing_path = executing_path_;

  std::transform(breakpoint_path.begin(), breakpoint_path.end(), breakpoint_path.begin(), ::tolower);
  std::transform(executing_path.begin(), executing_path.end(), executing_path.begin(), ::tolower);

  if(executing_path.find(breakpoint_path) != std::string::npos)
  {
    return true;
  }
  if(breakpoint_path.find(executing_path) != std::string::npos)
  {
    return true;
  }

  return false;
}

int diesel_lua_debugger_request_debugger_pause(lua_State* L) {
  lua_getglobal(L, "LRDB_SERVER");
  auto lrdb = (LRDBWrapper*)lua_touserdata(L, 1);
  lua_pop(L, 1);
  lrdb->lrdb->debugger_.pause();
  return 0;
}

LRDBWrapper* debug_server;
void setup_lrdb_on_state(lua_State* L) {

  //luaL_dostring(this_->L, "print(_G.setfenv)");

  if (debug_server->lrdb->debugger_.state_ != nullptr) debug_server->lrdb->reset(0);

  debug_server->lrdb->debugger_.set_breakpoint_path_handler(diesel_lua_debugger_breakpoint_path_handler);
  debug_server->lrdb->reset(L);

  lua_pushlightuserdata(L, debug_server);
  lua_setglobal(L, "LRDB_SERVER");

  lua_register(L, "__lrdb_pause", diesel_lua_debugger_request_debugger_pause);

  //debug_server->lrdb->command_stream().wait_for_connection();

  std::cout << "LRDB server started" << std::endl;

  if (debug_server->wait_for_debug) {
    std::cout << "Waiting for debugger to connect" << std::endl;
  }
}
dsl::LuaInterpreter* __fastcall dsl__LuaInterpreter__ctor_h(dsl::LuaInterpreter* this_, __int64 thread, bool libs, bool classes, bool main, int allocation) {
  dsl__LuaInterpreter__ctor_o(this_, thread, libs, classes, main, allocation);
  std::cout << "dsl::LuaInterpreter::ctor called " << this_->L << " libs: " << (libs ? "true" : "false") << " main: " << (main ? "true" : "false") << std::endl;

  if (main && libs) {
    setup_lrdb_on_state(this_->L);
  }

  return this_;
}
/*__int64 __fastcall dsl__LuaInterface__newstate_h(dsl::LuaInterface* this_, bool libs, bool main, int allocation) {
  __int64 retn = dsl__LuaInterface__newstate_o(this_, libs, main, allocation);

  std::cout << "dsl::LuaInterface::newstate called " << this_->L << " libs: " << (libs ? "true" : "false") << " main: " << (main ? "true" : "false") << std::endl;

  if (main && libs) {
    setup_lrdb_on_state(this_->L);
  }
  return retn;
}*/

__int64 __fastcall dsl__LuaInterface__dtor_h(dsl::LuaInterface* this_) {
  std::cout << "Lua state " << this_->L << " closed" << '\n';
  if(this_->L == debug_server->lrdb->debugger_.state_)
    debug_server->lrdb->reset(0);

  __int64 ret = dsl__LuaInterface__dtor_o(this_);


  return ret;
}

typedef struct StringReaderCtx {
  const char* str;
  size_t size;
} StringReaderCtx;
static const char* reader_string(lua_State* L, void* ud, size_t* size) {
  StringReaderCtx* ctx = (StringReaderCtx*)ud;
  UNUSED(L);
  if (ctx->size == 0) return NULL;
  *size = ctx->size;
  ctx->size = 0;
  return ctx->str;
}
int lua_loadbufferx(lua_State* L, const char* buf, size_t size, const char* name, const char* mode) {
  StringReaderCtx ctx;
  ctx.str = buf;
  ctx.size = size;
  return lua_loadx(L, reader_string, &ctx, name, mode);
}
int lua_loadbuffer(lua_State* L, const char* buf, size_t size, const char* name) {
  return lua_loadbufferx(L, buf, size, name, NULL);
}
int lua_loadstring_with_chunkname(lua_State* L, const char* s, const char* chunkname) {
  return lua_loadbuffer(L, s, strlen(s), chunkname);
}

__int64 __fastcall lua_load_h(lua_State* L, lua_Reader reader, dsl::Buffer* data, const char* chunkname) {
  std::string possible_lua_file = (std::filesystem::path("./lua") / chunkname).string() + ".lua";
  possible_lua_file = std::filesystem::absolute(possible_lua_file).string();
  __int64 status = -1;

  if (should_decompile_file(chunkname, possible_lua_file)) {
    std::size_t size = 0;
    auto data_ = reader(L, data, &size);
    std::vector<char> buff(data_, data_ + size);
    decompile_buffer(buff, possible_lua_file);
  }

  if (std::filesystem::exists(possible_lua_file)) {
    std::stringstream sstr;
    std::ifstream file(possible_lua_file);
    sstr << file.rdbuf();
    file.close();

    status = lua_loadstring_with_chunkname(L, sstr.str().c_str(), (std::string("@") + possible_lua_file).c_str());


  }
  else {
    status = lua_load_o(L, reader, data, chunkname);
  }

  if (status != 0) {
    std::cout << "Error compiling lua chunk " << chunkname << (status == LUA_ERRSYNTAX ? " syntax error" : " memory allocation error") << std::endl;
    
    const char* error = lua_tolstring(L, -1, NULL);

    std::cout << error << std::endl;
  }

  return status;
}

std::string md5_digest_to_str(const boost::uuids::detail::md5::digest_type& digest) {
  const auto digest_char = reinterpret_cast<const int*>(&digest);
  std::string digest_str;
  boost::algorithm::hex(digest_char, digest_char + (sizeof(boost::uuids::detail::md5::digest_type)/ sizeof(int)), std::back_inserter(digest_str));
  return digest_str;
}
// check for an updated/different raid executable, will usually change whenever the lua does
// lua will be force decompiled if raid's executable updates
void check_for_updated_raid() {
  std::string previous_md5 = "";

  if(std::filesystem::exists("./lua/dieselluadebugger_raidmd5.txt")) {
    std::stringstream sstr;
    std::ifstream other("./lua/dieselluadebugger_raidmd5.txt");
    sstr << other.rdbuf();
    other.close();
    previous_md5 = sstr.str();
  }

  std::ifstream exe(executable, std::ios::binary);
  exe.seekg(0, std::ios::end);
  size_t size = exe.tellg();
  exe.seekg(0, std::ios::beg);

  char* executable_buffer = new char[size];
  exe.read(executable_buffer, size);
  exe.close();

  boost::uuids::detail::md5 hash;
  boost::uuids::detail::md5::digest_type digest;
  hash.process_bytes(executable_buffer, size);
  hash.get_digest(digest);
  
  delete[] executable_buffer;

  std::string digest_str = md5_digest_to_str(digest);

  if (digest_str != previous_md5) {
    std::ofstream md5_out("./lua/dieselluadebugger_raidmd5.txt");
    md5_out << digest_str;
    md5_out.close();

    std::cout << "Game update detected will recompile the lua unless disabled, old md5: " << previous_md5 << " new md5: " << digest_str << std::endl;

    g_force_new_decompilation = true;
  }
}

void setup_debugger() {
  {
    auto cmdline = GetCommandLine();
    if(std::string(cmdline).find("--wait-for-lua-debugger") != std::string::npos) {
      g_wait_for_debugger = true;
    } else {
      g_wait_for_debugger = false;
    }
    if(std::string(cmdline).find("--console") != std::string::npos) {
      AllocConsole();
      freopen("CONOUT$", "w", stdout);
    }
    if(std::string(cmdline).find("--force-lua-decompilation") != std::string::npos) {
      g_force_new_decompilation = true;
    }
    else {
      g_force_new_decompilation = false;
    }
    if(std::string(cmdline).find("--stop-lua-updates") != std::string::npos) {
      g_never_update_lua = true;
    }
    else {
      g_never_update_lua = false;
    }
  }

  std::filesystem::create_directory("./lua");

  check_for_updated_raid();

  if (g_never_update_lua) g_force_new_decompilation = false;

  if (g_force_new_decompilation) {
    std::cout << "Forcing decomplication of all lua source files" << std::endl;
  }

  debug_server = new LRDBWrapper(g_wait_for_debugger, 21110);

  load_hashlist();

  scan_for_all_lua_functions(executable);

  lua_load = (lua_load_type)FindPattern(executable, lua_load_pattern, lua_load_mask);
  //dsl__LuaInterface__newstate = (dsl__LuaInterface__newstate_type)FindPattern(executable, dsl__LuaInterface__newstate_pattern, dsl__LuaInterface__newstate_mask);
  dsl__LuaInterpreter__ctor  = (dsl__LuaInterpreter__ctor_type)FindPattern(executable, dsl__LuaInterpreter__ctor_pattern, dsl__LuaInterpreter__ctor_mask);
  dsl__LuaInterface__dtor = (dsl__LuaInterface__dtor_type)FindPattern(executable, dsl__LuaInterface__dtor_pattern, dsl__LuaInterface__dtor_mask);
  dsl__idstring__t = (dsl__idstring__t_t)FindPattern(executable, dsl__idstring__t_pattern, dsl__idstring__t_mask);
  raid_string_assign = (raid_string_assign_t)FindPattern(executable, raid_string_assign_pattern, raid_string_assign_mask);

  MH_Initialize();

#define hook(name) \
  std::cout << #name << " CreateHook: " << MH_StatusToString(MH_CreateHook((LPVOID)name, name##_h, reinterpret_cast<LPVOID*>(&name##_o))) << std::endl;\
  std::cout << #name << " EnableHook: " << MH_StatusToString(MH_EnableHook((LPVOID)name)) << std::endl;

  hook(lua_load);
  hook(dsl__LuaInterpreter__ctor);
  hook(dsl__LuaInterface__dtor);
  hook(dsl__idstring__t);
}


BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved) { // reserved
  if (fdwReason == DLL_PROCESS_ATTACH) {
    setup_debugger();
  } else if (fdwReason == DLL_PROCESS_DETACH)
  {
    delete debug_server;
  }
  return TRUE;
}

// From RAID-BLT https://raw.githubusercontent.com/Luffyyy/Raid-BLT/master/src/signatures/signatures.cpp
MODULEINFO GetModuleInfo(std::string szModule) {
  MODULEINFO modinfo = { 0 };
  HMODULE hModule = GetModuleHandleA(szModule.c_str());
  if (hModule == 0)
    return modinfo;
  GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
  return modinfo;
}


unsigned __int64 FindPattern(std::string module, const char* pattern, const char* mask) {
  MODULEINFO mInfo = GetModuleInfo(module);
  DWORDLONG base = (DWORDLONG)mInfo.lpBaseOfDll;
  DWORDLONG size = (DWORDLONG)mInfo.SizeOfImage;
  DWORDLONG patternLength = (DWORDLONG)strlen(mask);
  for (DWORDLONG i = 0; i < size - patternLength; i++) {
    bool found = true;
    for (DWORDLONG j = 0; j < patternLength; j++) {
      found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
    }
    if (found) {
      //			printf("Found %s: 0x%p\n", funcname, base + i);
      return base + i;
    }
  }
  //MessageBoxA(NULL, std::format("Warning: Failed to locate function %s\n", funcname).c_str(), "FindPattern", NULL);
  return NULL;
}

