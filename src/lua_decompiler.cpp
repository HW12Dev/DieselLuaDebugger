#include "lua_decompiler.h"

#include <fstream>

//luajit-decompiler/main.h
#include "main.h"

bool g_force_new_decompilation = false;
extern bool g_never_update_lua;

bool should_decompile_file(const char* chunkname, std::filesystem::path possible_dest_path) {
  if (g_never_update_lua) return false;
  if (g_force_new_decompilation) return true;

  std::string chunk_str = chunkname;

  if (chunk_str.find(":") != std::string::npos) { // C:\
    // loading already plaintext file
    return false;
  }
  if (chunk_str.find("mods") != std::string::npos) { // file is a part of a mod, already plaintext
    return false;
  }
  if (chunk_str.find("@") != std::string::npos) { // file is loaded as part of lua_loadfile, and is already loose on the disk
    return false;
  }

  if (possible_dest_path.string().find("RAID World War II") && possible_dest_path.string().find("lua")) { // {install_dir}/lua
    if (!std::filesystem::exists(possible_dest_path)) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

void decompile_buffer(const std::vector<char>& buffer, std::filesystem::path destination) {
  Bytecode bc("", buffer);
  Ast ast(bc, false, true);
  Lua lua(bc, ast, destination.string(), true, true, true);

  bc();
  ast();
  lua();

  if(!std::filesystem::exists(destination.parent_path()))
    std::filesystem::create_directories(destination.parent_path());
  std::ofstream outfile(destination, std::ios::binary);
  outfile << lua.writeBuffer;
  outfile.close();
}
