$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot
try {
    & gcc -std=gnu99 -O2 -Wall -Wextra -Dmain=maze_main -Isrc -c src/maze.c -o tests/maze_test_main.o
    if ($LASTEXITCODE -ne 0) { throw 'maze.c test build failed' }

    $sources = @(
        'tests/game_logic_test.c', 'tests/maze_test_main.o',
        'src/map.c', 'src/solution.c', 'src/event.c', 'src/Interface.c',
        'src/file.c', 'src/filedialog.c', 'src/particle.c', 'src/enemy.c',
        'src/item.c', 'src/fog.c', 'src/level.c', 'src/audio.c',
        'src/save.c', 'src/fpview.c'
    )
    & gcc -std=gnu99 -O2 -Wall -Wextra -Isrc @sources -o tests/game_logic_test.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lshell32
    if ($LASTEXITCODE -ne 0) { throw 'game logic test build failed' }

    & tests/game_logic_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'game logic tests failed' }
} finally {
    Pop-Location
}
