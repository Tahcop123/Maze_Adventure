# Maze Adventure: Dungeon Edition

A top-down / first-person dungeon crawler written in **C** and built with
[raylib 5.5](https://www.raylib.com/). Started as a C-language course-design
project and modernized with procedural generation, turn-based combat, items,
portals, ice sliding, push-box puzzles, achievements, and binary save/load.

一个用 **C 语言** 编写、基于 [raylib 5.5](https://www.raylib.com/) 的地牢迷宫游戏。
最初是 C 语言课程设计作业，现已改造为带有程序化地图生成、回合制战斗、道具、
传送门、冰面滑行、推箱子机关、成就系统与二进制存档的完整游戏。



---

## Features / 特性

- **Two view modes / 双视角** — classic grid top-down, or real-time first-person 3D.
  经典网格俯视视角，也可随时切换到第一人称 3D 视角（按 `V`）。
- **Three difficulty levels / 三个难度** — Easy / Normal / Hard, different map size,
  enemy/item/trap counts; Hard has a 120-second timer.
  简单 / 普通 / 困难，地图尺寸、敌人、道具、陷阱数量各不相同，困难模式限时 120 秒。
- **Three characters / 三个角色** — Knight (+20 HP), Mage (wider vision),
  Rogue (10 free steps).
  骑士（额外 20 生命）、法师（更大视野）、盗贼（前 10 步不扣血）。
- **Procedural maze / 程序化迷宫** — random seed per run; `F2` rolls a new map.
  每局随机种子生成迷宫，按 `F2` 重开一张新地图。
- **Combat / 战斗** — melee attack in both views; slimes, patrolling skeletons,
  and ghosts that chase you and pass through walls (but not doors).
  双视角都可近战；史莱姆、巡逻骷髅、穿墙幽灵会主动追击。
- **Items / 道具** — bomb (3×3 wall blast), speed boots (10s free steps),
  shield (blocks one hit), torch (expanded vision).
  炸弹（3×3 炸墙）、加速鞋（10 步不扣血）、护盾（抵挡一次伤害）、火把（扩大视野）。
- **World mechanics / 世界机制** — spike traps, paired portals, ice slides,
  pushable boxes on pressure plates, hidden bonus rooms, colored keys opening
  matching doors.
  尖刺陷阱、成对传送门、冰面滑行、推箱子压板开门、隐藏奖励房间、彩色钥匙开对应颜色的门。
- **Progression / 进度系统** — local high-score table, 15 unlockable achievements,
  binary save/load.
  本地排行榜、15 个成就、二进制存档（暂停菜单 → Save / Load Game）。


---

## Build / 构建

Requires a working **raylib 5.5** and a C99 compiler on Windows.
Tested with **MSYS2 UCRT64 GCC 15.x** (add `D:\msys64\ucrt64\bin` to `PATH`).

需要 Windows 下可用的 **raylib 5.5** 与 C99 编译器，已在 **MSYS2 UCRT64 GCC 15.x**
（将 `D:\msys64\ucrt64\bin` 加入 `PATH`）上验证。

```bat
build.bat
```

For a logic regression check with the same MSYS2 GCC toolchain, run
`pwsh -File tests/run_tests.ps1` from this folder.
测试覆盖地图导入、彩门、隐藏房间、开局状态和按时间推进的敌人/陷阱。

Or manually / 或手动：

```bash
cd src
gcc -o ..\maze.exe -std=gnu99 -O2 -Wall -Wextra -I. ^
  maze.c map.c solution.c event.c Interface.c file.c filedialog.c ^
  particle.c enemy.c item.c fog.c level.c audio.c save.c fpview.c ^
  -lraylib -lopengl32 -lgdi32 -lwinmm -lcomdlg32 -lshell32
```

> **CRT note / 运行库说明**: the official prebuilt `libraylib.a` is UCRT-based.
> Linking it against an **MSVCRT-based** MinGW (e.g. mingw-builds 8.x) mixes
> runtimes and crashes at startup. Either use the MSYS2 UCRT64 toolchain
> (recommended), or link the DLL instead — `-lraylibdll` — and keep
> `raylib.dll` next to `maze.exe` (that is how the shipped binary is built).
> 官方预编译 `libraylib.a` 基于 UCRT；若用 MSVCRT 版 MinGW 静态链接会因混用
> 运行库启动即崩溃。请改用 MSYS2 UCRT64 工具链，或改链 DLL
> （`-lraylibdll`，并把 `raylib.dll` 放到 `maze.exe` 旁边）。

Then run `maze.exe`. High scores, achievements and the save file are written
next to the executable.

运行 `maze.exe` 即可。排行榜、成就、存档都保存在可执行文件同目录。

## How to play / 玩法说明

**Goal / 目标:** Grab the golden key, then reach the red flag.
Pick up coins and gems for extra score; avoid enemies and traps.

**目标：** 先拿到金色钥匙，再走到红色终点旗帜。沿途收集金币和宝石加分，躲开敌人和陷阱。

### Controls / 操作

| Key / 按键 | Action / 作用 |
| --- | --- |
| `WASD` / Arrow / 方向键 | Move player (top-down) / 俯视移动 |
| Mouse / 鼠标 | Look around (first-person) / 第一人称环视 |
| `Left Shift` / 左 Shift | Sprint, costs stamina / 冲刺，消耗体力 |
| `Space` / 空格 | Attack / 攻击 |
| `V` | Toggle first-person / top-down / 切换第一人称/俯视 |
| `1` `2` `3` `4` | Bomb / Speed / Shield / Torch / 用炸弹/加速/护盾/火把 |
| `M` | Toggle music / 音乐开关 |
| `Esc` or `P` / Esc 或 P | Pause menu / 暂停菜单 |
| `F2` | New random map / 随机新地图 |
| `F3` | Toggle map edit mode / 开关地图编辑 |
| `F5` | Save map (editor) / 保存地图（编辑模式） |
| `F9` | Open a saved map / 打开已有地图文件 |

### Game rules / 规则

- Each step costs 1 HP (free under Speed Boots or Rogue starting steps).
  每走一步扣 1 点血（加速鞋或盗贼初始免伤步内不扣）。
- Contact with an enemy costs damage equal to `attack - defense` (min 1);
  shield absorbs one hit. Ghosts chase along the longer axis and pass through
  walls but not colored doors.
  碰到敌人会按 `攻击力 - 防御力`（至少 1 点）扣血，护盾可挡一次；
  幽灵沿更远的轴追击，能穿墙但穿不过彩门。
- Ice tiles make you slide until you hit a wall.
  踩上冰面会一路滑到墙为止。
- Stand on a pressure plate with a box to open the matching colored door.
  把箱子推到压力板上可打开对应颜色的门。
- Paired portals teleport you between two fixed locations.
  成对传送门可在两个固定位置间瞬移。
- Hard mode finishes in 120 seconds or you lose.
  困难模式 120 秒内未通关即失败。

## Project layout / 目录结构

```
MazeGame/
├── README.md
├── build.bat               # one-click Windows build / 一键构建
├── src/
│   ├── *.c / *.h           # game source / 游戏源码
│   └── Linkedlist/         # unused legacy module (safe to delete) / 未使用的旧模块（可删除）
├── legacy/maze/            # original console course-design sources / 旧版源码
└── scripts/                # misc helper scripts / 辅助脚本
```

## Status / 当前状态

All blocking gameplay bugs from the original review are fixed. A four-phase
optimization pass has since been applied:

- **Rendering perf** — the static maze and fog-of-war layers are baked into
  RenderTextures and only re-baked when the map/fog changes; vignettes and the
  menu gradient are pre-baked textures; light glows use one shared radial
  gradient; the BFS solution is cached and recomputed only when stale; the
  wall linked-list was replaced by O(1) grid math.
- **Bug fixes** — ice slide moved in the wrong direction; audio now uses an
  8-voice mixer so two-tone effects play both notes; animations no longer run
  while paused; score accumulates across levels; screen-shake math is safe.
- **Robustness** — save files are fully staged and validated before loading
  (entity counts, player position, door positions); save/load reports success
  or failure in the HUD; file dialogs run at a clean frame boundary.
- **Cleanliness** — cell codes replaced by named constants, dead code removed,
  `snprintf` everywhere, `-Wall -Wextra` clean.

The `src/Linkedlist/` folder is no longer part of the build and can be deleted.

原审查报告的阻断性 bug 均已修复，并完成了一轮四阶段优化：

- **渲染性能** —— 静态迷宫层与战争迷雾烘焙为 RenderTexture，仅在地图/迷雾变化时
  重烘焙；暗角、菜单渐变改为预生成纹理；光晕共用一张径向渐变图；BFS 解法缓存，
  仅在失效时重算；墙壁链表替换为 O(1) 网格坐标计算。
- **Bug 修复** —— 冰面滑行方向错误；音频改为 8 声部混音器，双音效可完整播放；
  暂停时动画不再走动；跨关分数累计；震屏数学安全化。
- **健壮性** —— 读档前完整暂存并校验（实体数量、玩家坐标、门坐标）；
  存/读档结果在 HUD 反馈；文件对话框在帧边界统一处理。
- **代码整洁** —— 格子类型改用命名常量、清理死代码、统一 `snprintf`、
  `-Wall -Wextra` 无警告。

`src/Linkedlist/` 已不参与编译，可安全删除。

## License / 许可

Educational course project — free to study and fork for learning.

教学课程项目，欢迎学习与 fork。
