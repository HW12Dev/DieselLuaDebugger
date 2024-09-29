# python prebuild.py [GAME] [STEAMBUILDID]
# Set build id to 0 for the latest version
import sys

game = sys.argv[1] # Game to build for, e.g. RAID
build_version = sys.argv[2] # Steam build id for the version you are building for e.g. 15840791 (RAID U22.3)

generated_file = ""

if game == "RAID":
  generated_file += "#define RAIDWW2\n"
  if build_version != "0":
    generated_file += "#define RAID_VERSION " + build_version
  else:
    generated_file += "#define RAID_VERSION 99999999"

f = open("./src/generated_game_information.h", "w")
f.write(generated_file)
f.close()