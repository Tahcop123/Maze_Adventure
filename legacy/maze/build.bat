@echo off
REM maze project build script for MSYS2 gcc
cd /d "%~dp0"

gcc -o maze.exe ^
  -std=gnu89 -fcommon ^
  -I. -Ilibgraphics -ILinkedlist -IsimpleGUI ^
  -Wno-incompatible-pointer-types -Wno-int-conversion ^
  -Wno-pointer-to-int-cast -Wno-return-mismatch ^
  libgraphics/exceptio.c libgraphics/genlib.c libgraphics/graphics.c ^
  libgraphics/random.c libgraphics/simpio.c libgraphics/strlib.c ^
  Linkedlist/linkedlist.c simpleGUI/imgui.c ^
  maze.c map.c event.c Interface.c solution.c file.c ^
  -mwindows -lgdi32 -lwinmm -lole32 -luuid -lcomdlg32 -lshell32

if %errorlevel%==0 (
    echo.
    echo === BUILD SUCCESS ===
    echo Output: maze.exe
) else (
    echo.
    echo === BUILD FAILED ===
)
pause
