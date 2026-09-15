#include "file.h"
#include "map.h"
#include "filedialog.h"
#include <stdio.h>
#include <stdlib.h>

static void ShowError(const char *msg)
{
    (void)msg;
}

// Text map format (supports multi-digit cell codes 6/7/8, 10-12, 20/21):
//   line 1: "<rows> <cols>"
//   then rows x cols space-separated integers, indexed [1..Row][1..Col]
void Openfile(void)
{
    char filename[1024] = {0};
    if (!OpenFileDialog(filename, sizeof(filename))) return;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        ShowError("Failed to open file!");
        return;
    }

    int r = Row, c = Col;
    if (fscanf(fp, "%d %d", &r, &c) == 2 && r >= 5 && r <= 98 && c >= 5 && c <= 98) {
        Row = r;
        Col = c;
    } else {
        rewind(fp); // headerless/legacy file: fall back to current dimensions
    }

    // Reset to walls inside, -1 border
    for (int i = 0; i <= Row + 1; i++) {
        for (int j = 0; j <= Col + 1; j++) {
            int border = (i == 0 || j == 0 || i == Row + 1 || j == Col + 1);
            map_change[i][j] = border ? -1 : 3;
        }
    }

    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            int v;
            if (fscanf(fp, "%d", &v) == 1) map_change[i][j] = v;
        }
    }
    fclose(fp);

    // Keep the BFS working copy in sync
    for (int i = 0; i <= Row + 1; i++)
        for (int j = 0; j <= Col + 1; j++)
            map[i][j] = map_change[i][j];
}

void Savefile(void)
{
    char filename[1024] = {0};
    if (!SaveFileDialog(filename, sizeof(filename))) return;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        ShowError("Failed to save file!");
        return;
    }

    fprintf(fp, "%d %d\n", Row, Col);
    for (int i = 1; i <= Row; i++) {
        for (int j = 1; j <= Col; j++) {
            fprintf(fp, "%d%s", map_change[i][j], (j == Col) ? "\n" : " ");
        }
    }
    fclose(fp);
}
