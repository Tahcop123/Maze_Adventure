# MazeGame optimization iterations

The repository is a C/raylib project. The iterative-coder workflow uses GCC
`-Wall -Wextra` and the focused C regression runner in place of Python's
ruff/pytest checks.

## Iteration 1 — Map import
- **Goal:** Reject malformed maps without changing the current game; reset imported-map state only after a successful load.
- **Changes:** Staged parsing, strict format and cell validation, import result handling, spawn selection and imported collectible setup. Incomplete editor maps remain loadable.
- **Evaluation:** Final GCC build and map regression tests passed.

## Iteration 2 — First-person doors
- **Goal:** Make collected keys open doors in either view.
- **Changes:** Shared door-opening function and collision-edge check in first-person movement.
- **Evaluation:** Final GCC build and key/door regression test passed. Camera movement still needs an interactive check.

## Iteration 3 — Hidden room
- **Goal:** Preserve the start, golden key and exit when carving a bonus room.
- **Changes:** Reject candidate rooms overlapping special cells and award the room bonus on entry.
- **Evaluation:** Final generated-map tests passed for 50 seeds on each difficulty.

## Iteration 4 — Run state
- **Goal:** Reset run-only state on new game, retry and F2 while retaining inventory between levels.
- **Changes:** Central `ResetRunState` and per-level rogue, view, hint and stamina initialization.
- **Evaluation:** Final reset regression test and GCC build passed.

## Iteration 5 — Elapsed-time updates
- **Goal:** Make enemy and trap timing independent of display frame rate.
- **Changes:** Pass frame delta into both update functions and accumulate 60 Hz logical ticks.
- **Evaluation:** Final timing regression test and GCC build passed.

## Iteration 6 — Compatibility and edge cases
- **Feedback:** Requiring a key and exit would reject incomplete editor maps; one test expectation overlooked the ghost's axis-switching chase.
- **Changes:** Keep incomplete maps loadable, make pathfinding safe without an exit, process multiple enemy moves after a long frame and correct the test expectation.
- **Evaluation:** The revised regression suite passed. GCC compilation has no warnings.

Final checks: `pwsh -File tests/run_tests.ps1` passed; the normal game target
compiled with GCC `-Wall -Wextra` without warnings. Full interactive play was
not part of the automated checks.
