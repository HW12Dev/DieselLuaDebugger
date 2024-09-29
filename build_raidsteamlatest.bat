@REM Build script for RAID: World War II version u22.3
python signature_formatter.py
python prebuild.py RAID 15840791
xmake build DieselLuaDebugger
xmake build VERSION