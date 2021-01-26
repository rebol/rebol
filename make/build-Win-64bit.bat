make make OS_ID=0.3.40 REBOL_TOOL=prebuild\r3-make-win.exe
make TOOLS=x:/MingW/mingw64/bin/ all

SET BUILD_DIR=..\build\win-x64\
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
move /Y r3.exe     %BUILD_DIR%
move /Y host.exe   %BUILD_DIR%
move /Y libr3.dll  %BUILD_DIR%
@dir %BUILD_DIR%
@pause