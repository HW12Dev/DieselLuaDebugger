target("VERSION")
  set_kind("shared")
  add_files("./proxy.cpp")
  add_files("./version.def")
  set_languages("cxx20")
  
  add_rules("mode.release")