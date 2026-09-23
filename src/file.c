#include "file.h"
#include "map.h"
#include "filedialog.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static int ValidCell(int cell)
{
    return (cell >= CELL_ROAD && cell <= CELL_KEY_GREEN && cell != CELL_PATH) ||
           (cell >= CELL_DOOR_RED && cell <= CELL_DOOR_GREEN) ||
           cell == CELL_COIN || cell == CELL_GEM;
}

// Text map format (supports multi-digit cell codes 6/7/8, 10-12, 20/21):
//   line 1: "<rows> <cols>"
//   then rows x cols space-separated integers, indexed [1..Row][1..Col]
int LoadMapFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return 0;

    int r, c;
    if (fscanf(fp, "%d %d", &r, &c) != 2 || r < 5 || r > 98 || c < 5 || c > 98) {
        fclose(fp);
        return 0;
    }

    // Parse into a temporary grid; a malformed file cannot alter the live map.
    int staged[100][100];
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++) staged[i][j] = CELL_BORDER;

    int spawnable = 0, collectibles = 0;
    for (int i = 1; i <= r; i++) {
        for (int j = 1; j <= c; j++) {
            int v;
            if (fscanf(fp, "%d", &v) != 1 || !ValidCell(v)) {
                fclose(fp);
                return 0;
            }
            staged[i][j] = v;
            if (v == CELL_START || v == CELL_ROAD) spawnable++;
            if (v == CELL_COIN || v == CELL_GEM) collectibles++;
        }
    }
    int trailing;
    do { trailing = fgetc(fp); } while (trailing != EOF && isspace((unsigned char)trailing));
    fclose(fp);
    if (trailing != EOF || spawnable == 0 || collectibles > MAX_COINS) return 0;

    Row = r;
    Col = c;
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < 100; j++) map[i][j] = map_change[i][j] = staged[i][j];
    return 1;
}

int Openfile(void)
{
    char filename[1024] = {0};
    if (!OpenFileDialog(filename, sizeof(filename))) return 0;
    return LoadMapFile(filename);
}

void Savefile(void)
{
    char filename[1024] = {0};
    if (!SaveFileDialog(filename, sizeof(filename))) return;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) return;

    fprintf(fp, "%d %d\n", Row, Col);
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            fprintf(fp, "%d%s", map_change[i][j], (j == Col) ? "\n" : " ");
        }
    }
    fclose(fp);
}
