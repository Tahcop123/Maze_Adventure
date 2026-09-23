@echo off
setlocal
cd /d "%~dp0src"

echo === Building Maze Adventure - Dungeon Edition ===
echo.

gcc -o ..\maze.exe ^
  -std=gnu99 -O2 -Wall -Wextra ^
  -I. ^
  maze.c map.c solution.c event.c Interface.c file.c filedialog.c ^
  particle.c enemy.c item.c fog.c level.c audio.c save.c fpview.c ^
  -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lshell32

if %errorlevel%==0 (
    echo.
    echo === BUILD SUCCESS ===
    echo Output: maze.exe
) else (
    echo.
    echo === BUILD FAILED ===
)
pause
