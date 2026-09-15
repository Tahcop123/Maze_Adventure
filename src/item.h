#ifndef _ITEM_H
#define _ITEM_H

#define MAX_ITEMS 30

typedef enum {
    ITEM_BOMB = 0,
    ITEM_SPEED = 1,
    ITEM_SHIELD = 2,
    ITEM_TORCH = 3
} ItemType;

typedef struct {
    int x, y;
    int type;
    int active;
    float animTime;
} Item;

extern Item items[MAX_ITEMS];
extern int itemCount;

void InitItems(void);
void SpawnItem(int x, int y, int type);
void CheckItemPickup(void);
void UseItem(int type);
void DrawItems(void);
const char* ItemName(int type);

#endif
