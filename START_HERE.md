# Lone Dune: The Last Signal

A Dune-themed version of the Rover Expedition project.

## Run the game

In a terminal inside this folder:

```sh
bash tools/build.sh
```

On a Mac, you can also double-click **Run Lone Dune.command in Finder**.
Clicking it in VS Code opens the script for editing instead.
For the VS Code shortcut, open **Lone Dune.code-workspace** and press **Command + Shift + B**.
These all build the current code before running it.

To check the game rules without opening a window:

```sh
bash tools/check.sh
```

## Play

Click a clear square to choose a goal. The rover follows the letters in
`rover/missions.txt`; clicking does not tell it where to drive.
Passing through the goal counts, even if the rover moves past it.

| Control | What it does |
| --- | --- |
| Hover | Preview a goal square before the first command. |
| Click | Choose or change the goal before the first command. |
| Space | Run one command. |
| Enter | Run the remaining commands, one every 0.3 seconds. |
| N | Go to the next mission after this one finishes. |
| R | Restart the whole expedition with a fresh map and full battery. |
| Escape | Close the game. |
| Left / Right | Show other pages of a long command list. |

Pressing R also starts a new summary report. The rover keeps its position,
battery, and samples between missions when you use N.

| Mission | Suggested goal (column, row) | Route |
| --- | --- | --- |
| 1 | (4, 1) | Scan, collect a sample, and take a photo. |
| 2 | (3, 4) | Reach another charging station and recharge. |
| 3 | (9, 7) | Collect a second sample, take a photo, and recharge. X is rejected. |

Columns and rows start at 0. East adds to x; south adds to y.

## Find the code

Each `.h` file lists a class's variables and functions. Its `.cpp` file contains the function code.

| File in rover/ | What to look for |
| --- | --- |
| main.cpp | Read the files, handle clicks and keys, restart, and save results. |
| World.cpp | Read and check the map, find terrain, reveal and remove samples. |
| Rover.cpp | Move, turn, scan, drill, and check the battery. |
| Mission.cpp | Choose a goal, run the next command, and count mission results. |
| WorldView.cpp | Draw the map and rover, convert mouse coordinates, and animate moves. |
| Location.cpp | Store and compare x/y positions. |
| Direction.h | Name the four directions. |

Follow one F command: main notices Space, Mission selects the next letter,
Rover checks the move and battery, Mission records the result, and WorldView draws it.
Only main and WorldView include graphics headers.

| Folder | What is in it |
| --- | --- |
| rover/ | Game code, map, commands, rover artwork, and saved results. |
| tests/ | Rule checks in tests.cpp and small input files in data/. |
| library/ | The supplied graphics library. |
| tools/ | The current build and test scripts. |
| reference/ | Supplied instructions and old build scripts for the starter layout. |
| build/ | Compiled files. The scripts recreate these. |

Use the scripts in tools, not the old scripts in reference.

## Map and commands

`.` is ground, `#` is rock, `~` is sand, `C` is a charger, and `*` is a hidden sample.
A sample looks like ground until it is scanned. Drilling collects it and leaves ground.

| Command | Action | Battery cost |
| --- | --- | --- |
| F | Forward one square | 2, or 4 into sand |
| B | Backward one square, keeping the same heading | 3, or 5 into sand |
| L / R | Turn left / right | 1 |
| S | Scan the surrounding 3 by 3 squares, including the rover's square | 3 |
| D | Collect a revealed sample under the rover | 5 |
| P | Save a photo | 1 |
| C | Recharge to 40 at any charger | 0 |

Rejected commands still use a turn but do not use battery or change the map.
Moves check the map edges and rocks before checking battery. Drilling checks
for a revealed sample before checking battery. Unknown letters and spaces are rejected.

## Visual additions

- The rover uses four pictures in `rover/arrakis_rover.png`, one for each direction.
- Moves slide between squares over 0.22 seconds. A Space press during a slide waits for it to finish.
- A blue outline previews valid goal squares before the first command.

These use the supplied Image, Sprite, Rectangle, and Window classes.

## Saved results and submission

The game saves these in rover:

| File | When it is saved |
| --- | --- |
| mission_summary.csv | A new header at startup or restart, then one row per completed mission. |
| waiting_for_goal.png | Before choosing the first goal. |
| during_mission.png | During the mission, after a command finishes moving. |
| photo_<mission>_<command>.png | When a P command succeeds. Both numbers start at 1. |

Starting again clears the report, but old photos remain until replaced.
For submission, finish all three missions, close the game, and check that the
CSV has its header plus three result rows. Use screenshots from that run.
Do not restart the game before saving the final submission copy.

Submit one ZIP with the source and headers, supplied library, current build scripts,
world.txt, missions.txt, tests and their input files, completed CSV, and the three
required pictures: waiting for a goal, during a mission, and one rover photo.
Include arrakis_rover.png so the sprite can load. Leave out build/, tests/results/,
and compiled Mac apps. No submission ZIP has been made yet.

