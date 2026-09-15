#include "solution.h"
#include <stddef.h>

int is_showsolution = 0;
int is_showtip = 0;
int shortstep = 1;
int xx, yy;

struct location entry, destination, current[10000];
int pre[10000];
int visit[100][100] = {0};

int can_go(int x, int y)
{
    if (x > 0 && x <= Row && y > 0 && y <= Col) {
        int v = map[x][y];
        int walkable = (v == 0 || v == 2 || v == 5 || v == 4 ||
                        (v >= 6 && v <= 8) || v == 20 || v == 21);
        if (walkable && !visit[x][y]) return 1;
    }
    return 0;
}

int BFSPath(int sx, int sy, int tx, int ty, int *next_x, int *next_y)
{
    int i;
    int head = 0, tail = 0;
    current[head].x = sx;
    current[head].y = sy;
    pre[head] = -1;
    visit[sx][sy] = 1;

    while (head <= tail) {
        for (i = 0; i < 4; i++) {
            int x = current[head].x + direction[i][0];
            int y = current[head].y + direction[i][1];
            if (can_go(x, y)) {
                tail++;
                current[tail].x = x;
                current[tail].y = y;
                pre[tail] = head;
                visit[x][y] = 1;
                if (x == tx && y == ty) {
                    int t = tail, node_count = 0, first_t = -1;
                    while (t != -1) {
                        if (pre[t] != -1) {
                            if (!(current[t].x == tx && current[t].y == ty))
                                map[current[t].x][current[t].y] = 1;
                            if (pre[t] == 0) first_t = t;
                        }
                        node_count++;
                        t = pre[t];
                    }
                    if (first_t != -1 && next_x && next_y) {
                        *next_x = current[first_t].x;
                        *next_y = current[first_t].y;
                    }
                    return node_count - 1;
                }
            }
        }
        head++;
    }
    return -1;
}

void OptimalSolution(void)
{
    int i, j, m, n;
    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++) {
            if (map_change[i][j] != 1)
                map[i][j] = map_change[i][j];
            else
                map[i][j] = 0;
        }

    entry.x = X;
    entry.y = Y;
    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            if (map[i][j] == 5) {
                destination.x = i;
                destination.y = j;
            }

    int kx = -1, ky = -1;
    for (i = 1; i <= Row; i++)
        for (j = 1; j <= Col; j++)
            if (map[i][j] == 2) {
                kx = i;
                ky = j;
            }

    shortstep = 0;
    xx = entry.x;
    yy = entry.y;

    if (!is_key && kx > 0) {
        int nx, ny;
        int s1 = BFSPath(entry.x, entry.y, kx, ky, &nx, &ny);
        if (s1 >= 0) {
            xx = nx;
            yy = ny;
            for (m = 0; m < 100; m++)
                for (n = 0; n < 100; n++)
                    visit[m][n] = 0;
            int s2 = BFSPath(kx, ky, destination.x, destination.y, NULL, NULL);
            if (s2 >= 0)
                shortstep = s1 + s2;
        }
    } else {
        int s = BFSPath(entry.x, entry.y, destination.x, destination.y, &xx, &yy);
        if (s >= 0) shortstep = s;
    }

    for (m = 0; m < 100; m++)
        for (n = 0; n < 100; n++)
            visit[m][n] = 0;
}
