import json

type_definitions_f = open("./function_typedefinitiondata.json", "r")
type_definitions = json.load(type_definitions_f)
type_definitions_f.close()

signatures_f = open("./signature_dump.txt", "r")
signatures = signatures_f.readlines()
signatures_f.close()

functions = {}

for signature_line in signatures:
  function_entry = {}

  signature_line = signature_line.replace("\n", "")
  if signature_line == "" or signature_line.startswith("//"):
    continue

  name = signature_line.split(",")[0]
  pattern_data = signature_line.split(",")[1]

  function_entry["name"] = name
  if pattern_data.count(" ") > 1: # Signature left function scope
    # function has been inlined and needs manual implementation
    function_entry["needs_manual_implementation"] = True
  else:
    pattern = pattern_data.split(" ")[0]
    mask = pattern_data.split(" ")[1]

    function_entry["pattern"] = pattern
    function_entry["mask"] = mask
    function_entry["needs_manual_implementation"] = False
  
  functions[name] = function_entry

for function_data in type_definitions:
  functions[function_data["name"]]["return_type"] = function_data["return_type"]
  functions[function_data["name"]]["arguments"] = function_data["arguments"]

generated_file = "#include \"luajit_types.h\"\n"

for func_name in functions:
  function_data = functions[func_name]

  if function_data.get("return_type", "") == "":
    continue

  if function_data["needs_manual_implementation"]:
    
    generated_file += "extern {} {}({});\n".format(function_data["return_type"], function_data["name"], function_data["arguments"])
  else:
    generated_file += "LUA_FUNCTION_({}, {}, \"{}\", \"{}\", CONCAT_ARGS({}));\n".format(function_data["name"], function_data["return_type"], function_data["pattern"], function_data["mask"], function_data["arguments"])

out_file = open("./src/generated_lua_definitions.txt", "w")
out_file.write(generated_file)
out_file.close()