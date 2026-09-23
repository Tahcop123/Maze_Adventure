#include "inc.h"
#include <raymath.h>
#include <math.h>

static const float EYE_HEIGHT = 0.55f;
static const float PLAYER_RADIUS = 0.22f;
static const int RENDER_DIST = 18;

static Texture2D brickTex = {0};
static Texture2D floorTex = {0};
static Texture2D ceilTex = {0};
static int texturesInited = 0;
static Mesh cubeMesh = {0};
static Material brickMat = {0};
static int meshInited = 0;

static RenderTexture2D sceneRT = {0};
static RenderTexture2D brightRT = {0};
static RenderTexture2D blurRT1 = {0};
static RenderTexture2D blurRT2 = {0};
static Shader brightShader = {0};
static Shader blurShader = {0};
static int bloomInited = 0;
static int brightThresholdLoc = -1;
static int blurDirLoc = -1;
static int blurResLoc = -1;

static void AddNoise(Image *img, int amount)
{
    for (int y = 0; y < img->height; y++)
        for (int x = 0; x < img->width; x++) {
            Color *c = (Color*)((unsigned char*)img->data + (y * img->width + x) * 4);
            int n = (rand() % (amount*2+1)) - amount;
            c->r = (unsigned char)(c->r+n<0?0:(c->r+n>255?255:c->r+n));
            c->g = (unsigned char)(c->g+n<0?0:(c->g+n>255?255:c->g+n));
            c->b = (unsigned char)(c->b+n<0?0:(c->b+n>255?255:c->b+n));
        }
}

static void InitTextures(void)
{
    if (texturesInited) return;
    Image brick = GenImageColor(128, 128, (Color){125,110,95,255});
    for (int y = 0; y < 128; y += 32)
        ImageDrawRectangle(&brick, 0, y, 128, 3, (Color){60,55,50,255});
    for (int row = 0; row < 4; row++) {
        int xOff = (row%2==0)?0:32;
        for (int x = xOff; x < 128; x += 64)
            ImageDrawRectangle(&brick, x, row*32+3, 3, 29, (Color){60,55,50,255});
    }
    AddNoise(&brick, 12);
    brickTex = LoadTextureFromImage(brick);
    UnloadImage(brick);

    Image floor = GenImageColor(128, 128, (Color){50,46,42,255});
    for (int y = 0; y < 128; y += 32) ImageDrawRectangle(&floor, 0, y, 128, 2, (Color){30,28,25,255});
    for (int x = 0; x < 128; x += 32) ImageDrawRectangle(&floor, x, 0, 2, 128, (Color){30,28,25,255});
    AddNoise(&floor, 8);
    floorTex = LoadTextureFromImage(floor);
    UnloadImage(floor);

    Image ceil = GenImageColor(64, 64, (Color){32,30,28,255});
    AddNoise(&ceil, 6);
    ceilTex = LoadTextureFromImage(ceil);
    UnloadImage(ceil);

    cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    brickMat = LoadMaterialDefault();
    SetMaterialTexture(&brickMat, MATERIAL_MAP_DIFFUSE, brickTex);
    meshInited = 1;
    texturesInited = 1;
}

static const char *brightFS =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float threshold;\n"
    "void main() {\n"
    "  vec4 c = texture(texture0, fragTexCoord);\n"
    "  float b = dot(c.rgb, vec3(0.2126,0.7152,0.0722));\n"
    "  if (b > threshold) finalColor = c*fragColor; else finalColor = vec4(0.0);\n"
    "}\n";

static const char *blurFS =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "out vec4 finalColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 resolution;\n"
    "uniform vec2 direction;\n"
    "void main() {\n"
    "  vec4 sum = vec4(0.0); vec2 texel = 1.0/resolution;\n"
    "  sum += texture(texture0, fragTexCoord) * 0.227027;\n"
    "  sum += texture(texture0, fragTexCoord + direction*texel*1.0) * 0.1945946;\n"
    "  sum += texture(texture0, fragTexCoord - direction*texel*1.0) * 0.1945946;\n"
    "  sum += texture(texture0, fragTexCoord + direction*texel*2.0) * 0.1216216;\n"
    "  sum += texture(texture0, fragTexCoord - direction*texel*2.0) * 0.1216216;\n"
    "  sum += texture(texture0, fragTexCoord + direction*texel*3.0) * 0.054054;\n"
    "  sum += texture(texture0, fragTexCoord - direction*texel*3.0) * 0.054054;\n"
    "  finalColor = sum*fragColor;\n"
    "}\n";

static void InitBloom(int w, int h)
{
    if (bloomInited) return;
    sceneRT = LoadRenderTexture(w, h);
    brightRT = LoadRenderTexture(w/2, h/2);
    blurRT1 = LoadRenderTexture(w/2, h/2);
    blurRT2 = LoadRenderTexture(w/2, h/2);
    brightShader = LoadShaderFromMemory(0, brightFS);
    blurShader = LoadShaderFromMemory(0, blurFS);
    brightThresholdLoc = GetShaderLocation(brightShader, "threshold");
    blurDirLoc = GetShaderLocation(blurShader, "direction");
    blurResLoc = GetShaderLocation(blurShader, "resolution");
    float th = 0.55f;
    SetShaderValue(brightShader, brightThresholdLoc, &th, SHADER_UNIFORM_FLOAT);
    Vector2 halfRes = {(float)(w/2), (float)(h/2)};
    SetShaderValue(blurShader, blurResLoc, &halfRes, SHADER_UNIFORM_VEC2);
    bloomInited = 1;
}

void InitFPView(void)
{
    fpPosX = (float)(Y+0.5f); fpPosY = (float)(X+0.5f);
    fpDirX = 0; fpDirY = -1; fpPlaneX = 0.66f; fpPlaneY = 0;
    fpMouseLook = 1; DisableCursor();
    InitTextures();
    InitBloom(GetScreenWidth(), GetScreenHeight());
}

void RotateView(float angle)
{
    float od = fpDirX;
    fpDirX = fpDirX*cosf(angle) - fpDirY*sinf(angle);
    fpDirY = od*sinf(angle) + fpDirY*cosf(angle);
    float op = fpPlaneX;
    fpPlaneX = fpPlaneX*cosf(angle) - fpPlaneY*sinf(angle);
    fpPlaneY = op*sinf(angle) + fpPlaneY*cosf(angle);
}

void ToggleFPMode(void)
{
    fpMode = !fpMode;
    if (fpMode) InitFPView();
    else {
        X = (int)fpPosY; Y = (int)fpPosX;
        if (X < 1) X = 1;
        if (X > Row) X = Row;
        if (Y < 1) Y = 1;
        if (Y > Col) Y = Col;
        fpMouseLook = 0; EnableCursor();
    }
}

static int IsWallCell(int mx, int my)
{
    if (mx<0||mx>=100||my<0||my>=100) return 1;
    int v = map_change[my][mx];
    return (v==CELL_WALL||v==CELL_BORDER||(v>=CELL_DOOR_RED&&v<=CELL_DOOR_GREEN));
}

static int CircleCollides(float cx, float cy, float r)
{
    int minX=(int)(cx-r)-1, maxX=(int)(cx+r)+1;
    int minY=(int)(cy-r)-1, maxY=(int)(cy+r)+1;
    for (int i=minY;i<=maxY;i++)
        for (int j=minX;j<=maxX;j++) {
            if (!IsWallCell(j,i)) continue;
            float cx2=fmaxf((float)j,fminf(cx,(float)(j+1)));
            float cy2=fmaxf((float)i,fminf(cy,(float)(i+1)));
            float dx=cx-cx2, dy=cy-cy2;
            if (dx*dx+dy*dy<r*r) return 1;
        }
    return 0;
}

static void TryMove(float dx, float dy)
{
    // Unlock a door as soon as the player's collision circle reaches it.
    // Checking the centre alone never works: the circle stops before crossing
    // the cell boundary, so the target cell would remain the current cell.
    if (dx != 0) {
        float edgeX = fpPosX + dx + (dx > 0 ? PLAYER_RADIUS : -PLAYER_RADIUS);
        OpenDoorIfUnlocked((int)(fpPosY + dy), (int)edgeX);
    }
    if (dy != 0) {
        float edgeY = fpPosY + dy + (dy > 0 ? PLAYER_RADIUS : -PLAYER_RADIUS);
        OpenDoorIfUnlocked((int)edgeY, (int)(fpPosX + dx));
    }
    if (!CircleCollides(fpPosX+dx, fpPosY+dy, PLAYER_RADIUS)) {
        fpPosX+=dx; fpPosY+=dy; return;
    }
    if (dx!=0 && !CircleCollides(fpPosX+dx, fpPosY, PLAYER_RADIUS)) fpPosX+=dx;
    if (dy!=0 && !CircleCollides(fpPosX, fpPosY+dy, PLAYER_RADIUS)) fpPosY+=dy;
}

void UpdateFPView(float dt)
{
    if (!fpMode) return;
    Vector2 md = GetMouseDelta();
    if (md.x!=0) RotateView(md.x*0.0025f);
    float rs = 2.0f*dt;
    if (IsKeyDown(KEY_Q)) RotateView(-rs);
    if (IsKeyDown(KEY_E)) RotateView(rs);
    if (IsKeyDown(KEY_LEFT)) RotateView(-rs*1.5f);
    if (IsKeyDown(KEY_RIGHT)) RotateView(rs*1.5f);

    float sp = fpMoveSpeed*dt;
    if (IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT)) sp*=1.6f;
    float mx=0,my=0;
    if (IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)) { mx+=fpDirX*sp; my+=fpDirY*sp; }
    if (IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN)) { mx-=fpDirX*sp*0.7f; my-=fpDirY*sp*0.7f; }
    if (IsKeyDown(KEY_A)) { mx+=fpDirY*sp*0.8f; my-=fpDirX*sp*0.8f; }
    if (IsKeyDown(KEY_D)) { mx-=fpDirY*sp*0.8f; my+=fpDirX*sp*0.8f; }

    if (mx!=0||my!=0) {
        TryMove(mx,my);
        int gx=(int)fpPosY, gy=(int)fpPosX;
        if (gx!=X||gy!=Y) {
            X=gx; Y=gy; step++; UpdateFog();
            if (speedBoostTime<=0 && rogueFreeSteps<=0) {
                Hp--;
                if (Hp<=0) { gameOverReason=0; gameState=STATE_GAMEOVER; PlayLoseSound(); }
            }
            if (rogueFreeSteps>0) rogueFreeSteps--;
            // Shared cell interactions (key/items/traps/portals/...) so FP can win
            OnEnterCell();
            // A portal may relocate the grid cell: resync the FP position
            if (X!=gx || Y!=gy) {
                fpPosX=(float)(Y+0.5f); fpPosY=(float)(X+0.5f);
            }
        }
    }
}

static int IsInViewCone(float cellX, float cellZ, float fovDeg)
{
    float dx=cellX-fpPosX, dz=cellZ-fpPosY;
    float dist2=dx*dx+dz*dz;
    if (dist2>(float)RENDER_DIST*RENDER_DIST) return 0;
    if (dist2<4.0f) return 1;
    float dist=sqrtf(dist2);
    float dot=(dx*fpDirX+dz*fpDirY)/dist;
    return dot > cosf(fovDeg*DEG2RAD*0.6f);
}

void DrawFPView(void)
{
    // The FP renderer reads cached key/exit/bonus positions which are filled
    // during the maze bake - make sure that has happened at least once.
    EnsureMapCaches();
    int w=GetScreenWidth(), h=GetScreenHeight();
    static float bobTime=0;
    bool moving = IsKeyDown(KEY_W)||IsKeyDown(KEY_S)||IsKeyDown(KEY_A)||IsKeyDown(KEY_D)||
                   IsKeyDown(KEY_UP)||IsKeyDown(KEY_DOWN);
    if (moving) bobTime+=GetFrameTime()*10.0f; else bobTime*=0.9f;
    float bobY=sinf(bobTime)*0.03f, bobX=cosf(bobTime*0.5f)*0.015f;

    Camera3D cam={0};
    cam.position=(Vector3){fpPosX+bobX, EYE_HEIGHT+bobY, fpPosY};
    cam.target=(Vector3){fpPosX+fpDirX+bobX, EYE_HEIGHT+bobY, fpPosY+fpDirY};
    cam.up=(Vector3){0,1,0}; cam.fovy=70.0f; cam.projection=CAMERA_PERSPECTIVE;

    BeginTextureMode(sceneRT);
    ClearBackground((Color){8,8,12,255});
    BeginMode3D(cam);

    // Floor & ceiling (colored planes)
    DrawPlane((Vector3){Col/2.0f,0,Row/2.0f}, (Vector2){(float)Col+2,(float)Row+2}, (Color){48,44,40,255});
    DrawPlane((Vector3){Col/2.0f,1.0f,Row/2.0f}, (Vector2){(float)Col+2,(float)Row+2}, (Color){30,28,26,255});

    // Walls with frustum culling + brick texture via Mesh
    for (int i=1;i<=Row;i++)
        for (int j=1;j<=Col;j++) {
            int v=map_change[i][j];
            if (v!=CELL_WALL&&v!=CELL_BORDER&&!(v>=CELL_DOOR_RED&&v<=CELL_DOOR_GREEN)) continue;
            if (!IsInViewCone((float)j+0.5f,(float)i+0.5f,75.0f)) continue;
            Vector3 pos={(float)j+0.5f,0.5f,(float)i+0.5f};
            if (v==CELL_WALL||v==CELL_BORDER) {
                DrawMesh(cubeMesh, brickMat, MatrixTranslate(pos.x,pos.y,pos.z));
            } else if (v==CELL_DOOR_RED) DrawCube(pos,1,1,1,(Color){180,80,70,255});
            else if (v==CELL_DOOR_BLUE) DrawCube(pos,1,1,1,(Color){70,100,180,255});
            else DrawCube(pos,1,1,1,(Color){70,150,80,255});
        }

    // Key (cached position instead of a full-map scan)
    if (!is_key && xk >= 1) {
        float b=sinf(GetTime()*3.0f)*0.1f;
        DrawCube((Vector3){yk+0.5f,0.4f+b,xk+0.5f},0.3f,0.3f,0.3f,GOLD);
        DrawCubeWires((Vector3){yk+0.5f,0.4f+b,xk+0.5f},0.35f,0.35f,0.35f,YELLOW);
    }

    // End flag (cached position)
    if (endCellX >= 1) {
        DrawCube((Vector3){endCellY+0.5f,0.5f,endCellX+0.5f},0.2f,1.0f,0.2f,(Color){180,40,40,255});
        DrawCube((Vector3){endCellY+0.5f,0.85f,endCellX+0.5f},0.5f,0.3f,0.05f,(Color){220,50,50,255});
    }

    // Enemies
    for (int i=0;i<enemyCount;i++) {
        if (!enemies[i].active) continue;
        float ex=enemies[i].y+0.5f, ez=enemies[i].x+0.5f;
        float dist=sqrtf((ex-fpPosX)*(ex-fpPosX)+(ez-fpPosY)*(ez-fpPosY));
        if (dist>RENDER_DIST) continue;
        float b=sinf(GetTime()*4.0f+i)*0.08f;
        if (enemies[i].type==0) DrawSphere((Vector3){ex,0.25f+b,ez},0.3f,(Color){60,180,70,255});
        else if (enemies[i].type==1) {
            DrawSphere((Vector3){ex,0.5f+b,ez},0.25f,(Color){220,220,210,255});
            DrawCube((Vector3){ex,0.25f,ez},0.2f,0.3f,0.15f,(Color){220,220,210,255});
        } else DrawSphere((Vector3){ex,0.5f+b,ez},0.3f,(Color){180,180,220,200});
        if (enemies[i].isHurt) DrawSphereWires((Vector3){ex,0.4f,ez},0.4f,8,8,(Color){255,80,80,200});
    }

    // Legacy map-file collectibles (cached positions)
    for (int i=0;i<bonusCellCount;i++) {
        if (bonusCells[i].kind==CELL_COIN) {
            float b=sinf(GetTime()*3+bonusCells[i].x+bonusCells[i].y)*0.08f;
            DrawSphere((Vector3){bonusCells[i].y+0.5f,0.2f+b,bonusCells[i].x+0.5f},0.12f,GOLD);
        } else {
            float b=sinf(GetTime()*2.5f+bonusCells[i].x+bonusCells[i].y)*0.1f;
            DrawCube((Vector3){bonusCells[i].y+0.5f,0.25f+b,bonusCells[i].x+0.5f},0.15f,0.2f,0.15f,(Color){100,200,255,255});
        }
    }

    EndMode3D();
    EndTextureMode();

    // === Bloom ===
    BeginTextureMode(brightRT);
    BeginShaderMode(brightShader);
    DrawTextureRec(sceneRT.texture,(Rectangle){0,0,(float)w,(float)-h},(Vector2){0,0},WHITE);
    EndShaderMode();
    EndTextureMode();

    Vector2 dirH={1,0}, dirV={0,1};
    BeginTextureMode(blurRT1);
    BeginShaderMode(blurShader);
    SetShaderValue(blurShader,blurDirLoc,&dirH,SHADER_UNIFORM_VEC2);
    DrawTextureRec(brightRT.texture,(Rectangle){0,0,(float)(w/2),(float)-(h/2)},(Vector2){0,0},WHITE);
    EndShaderMode();
    EndTextureMode();

    BeginTextureMode(blurRT2);
    BeginShaderMode(blurShader);
    SetShaderValue(blurShader,blurDirLoc,&dirV,SHADER_UNIFORM_VEC2);
    DrawTextureRec(blurRT1.texture,(Rectangle){0,0,(float)(w/2),(float)-(h/2)},(Vector2){0,0},WHITE);
    EndShaderMode();
    EndTextureMode();

    BeginTextureMode(blurRT1);
    BeginShaderMode(blurShader);
    SetShaderValue(blurShader,blurDirLoc,&dirH,SHADER_UNIFORM_VEC2);
    DrawTextureRec(blurRT2.texture,(Rectangle){0,0,(float)(w/2),(float)-(h/2)},(Vector2){0,0},WHITE);
    EndShaderMode();
    EndTextureMode();

    BeginTextureMode(blurRT2);
    BeginShaderMode(blurShader);
    SetShaderValue(blurShader,blurDirLoc,&dirV,SHADER_UNIFORM_VEC2);
    DrawTextureRec(blurRT1.texture,(Rectangle){0,0,(float)(w/2),(float)-(h/2)},(Vector2){0,0},WHITE);
    EndShaderMode();
    EndTextureMode();

    // Composite scene + bloom
    DrawTextureRec(sceneRT.texture,(Rectangle){0,0,(float)w,(float)-h},(Vector2){0,0},WHITE);
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(blurRT2.texture,
        (Rectangle){0,0,(float)(w/2),(float)-(h/2)},
        (Rectangle){0,0,(float)w,(float)h},
        (Vector2){0,0},0,(Color){255,255,255,160});
    EndBlendMode();

    // Vignette
    for (int y=0;y<h;y+=4)
        for (int x=0;x<w;x+=4) {
            float dx=(x-w/2.0f)/(w/2.0f), dy=(y-h/2.0f)/(h/2.0f);
            float dist=sqrtf(dx*dx+dy*dy);
            if (dist>0.5f) {
                float a=(dist-0.5f)*1.3f; if (a>0.75f) a=0.75f;
                DrawRectangle(x,y,4,4,(Color){0,0,0,(unsigned char)(a*255)});
            }
        }

    // Crosshair
    DrawCircle(w/2,h/2,5,(Color){255,255,255,200});
    DrawCircle(w/2,h/2,2,(Color){0,0,0,220});
    DrawLine(w/2-10,h/2,w/2-5,h/2,(Color){255,255,255,180});
    DrawLine(w/2+5,h/2,w/2+10,h/2,(Color){255,255,255,180});
    DrawLine(w/2,h/2-10,w/2,h/2-5,(Color){255,255,255,180});
    DrawLine(w/2,h/2+5,w/2,h/2+10,(Color){255,255,255,180});

    // Weapon
    float wx=w-170+sinf(bobTime*0.5f)*6, wy=h-135+cosf(bobTime*0.7f)*4;
    if (playerAttacking) { float t=1.0f-(attackAnimTime/0.25f); wx+=sinf(t*PI)*50; wy-=sinf(t*PI)*40; }
    DrawRectangle((int)wx,(int)wy,10,90,(Color){190,190,200,255});
    DrawRectangle((int)wx-3,(int)wy,16,8,(Color){220,220,230,255});
    DrawRectangle((int)wx-5,(int)(wy+85),20,15,(Color){120,80,40,255});
    DrawRectangle((int)wx-2,(int)(wy+95),14,10,(Color){150,100,50,255});

    DrawText("WASD: Move | Mouse: Look | Shift: Sprint | Space: Attack | V: Exit FP", 20, h-30, 14, (Color){200,200,210,180});
}
