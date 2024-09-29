local projectName = "DieselLuaDebugger"

includes("version")
includes("dependencies/luajit-decompiler")

add_requires("minhook v1.3.3")
add_requires("boost 1.84.0")

add_rules("mode.release", "mode.debug")
--set_config("arch", "x86")

target(projectName)
  set_kind("shared")
  set_languages("cxx20")
  set_exceptions("cxx")

  set_symbols("debug")

  add_defines("LRDB_USE_BOOST_ASIO")

  add_includedirs("./src")
  add_includedirs("./dependencies/LRDB/include")
  add_includedirs("./dependencies/LRDB/include/lrdb")
  --add_includedirs("./dependencies/LRDB/third_party/asio/asio/include")
  add_includedirs("./dependencies/LRDB/third_party/picojson")

  add_files("./src/*.cpp")

  add_packages("minhook")
  add_packages("boost")
  
  add_deps("luajit-decompiler")
  add_includedirs("./dependencies/luajit-decompiler/", "./dependencies/luajit-decompiler/ast", "./dependencies/luajit-decompiler/bytecode", "./dependencies/luajit-decompiler/lua")

  --add_defines("RAIDWW2")

  --add_defines("BEFORE_RAID_15703546", "RAIDWW2")

  --add_includedirs("./dependencies/luajit/luajit/src")

  --add_links("libluajit")
  --add_linkdirs("./dependencies/luajit/luajit/src")

target_end()