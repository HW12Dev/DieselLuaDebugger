# DieselLuaDebugger
Mod that adds [LRDB (Lua Remote DeBugger)](https://github.com/satoren/LRDB/) support to release versions
of the Diesel Game Engine (PAYDAY: The Heist, PAYDAY 2, RAID: World War II)

Currently only works with [RAID World War II](https://store.steampowered.com/app/414740/RAID_World_War_II/)

This mod is still very experimental.

## Features
 - Lua Remote DeBugger
 - Pause the game's lua at any time to inspect/modify variables in what is running
 - idstring resolving*
 - Loads base game lua from raw text files*
 - Decompiles lua automatically for use with debugging
 - Supports [RAID-BLT](https://modworkshop.net/mod/21065)

*Hashlist must be provided

### Command Line Arguments
The following command line arguments can be passed to `raid_win64_release.exe`
 - `--console`: enable a debug console, very useful
 - `--wait-for-lua-debugger`: make the game pause until a lua debugger is connected
 - `--force-lua-decompilation`: force all lua to be re-decompiled
 - `--stop-lua-updates`: disable lua decompilation

## Usage
 - Install the mod using the instructions below
 - Use Visual Studio Code (with a folder open in either the game's install directory or `{install_dir}/lua`) to connect to the game
   - The game destroys it's lua instance whenever a level is loaded, so you will need to reconnect your debugger when that happens

## Installing
 - Install Visual Studio Code
 - Install the following extensions: https://marketplace.visualstudio.com/items?itemName=satoren.lrdb, https://marketplace.visualstudio.com/items?itemName=beamng.lrdb-beamng
 - Place `VERSION.dll` and `DieselLuaDebugger.dll` in your RAID install folder
 - Place a hashlist file named `hashlist.txt` or `hashlist` in the game's install folder

## Uninstalling
 - Delete `VERSION.dll`, `DieselLuaDebugger.dll` and the `{install_dir}/lua` folder

## Compiling
 - Install [xmake](https://xmake.io)
 - Run `project.bat`
 - Compile using Visual Studio 2022