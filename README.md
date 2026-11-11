# Wasp vs. Bee Game

A two-player, turn-based strategy game in C where a bee colony and a hornet colony compete on an 18 x 12 grid. Each side produces units, moves them, fights, and (for the bees) collects pollen.

Final project for the C programming course (2023-24), written in French. Two-person project:

- **Xiang Cui**: terminal version of the game, unit movement, unit production, combat and save files.
- **Yunliang Cao**: integration and the graphical version with the MLV library, support for multiple colonies, and bug fixing.

## Features

- Bee side: hives, queen, workers, warriors and squadrons, plus pollen collection.
- Hornet side: nests, queen and hornets.
- Several hives or nests per side.
- Saving a game and loading it again.
- Graphical interface driven by the mouse.

## Build

Linux (Debian or Ubuntu) with `gcc` or `clang` and the [MLV library](http://www-igm.univ-mlv.fr/~boussica/mlv/).

```sh
gcc "final_project_CUI Xiang_CAO Yunliang.c" -o game -lMLV
```

## Run

```sh
./game            # start a new game
./game savefile   # load a saved game
```

In the window, hold the left mouse button on a unit, drag to an empty cell and release.

## Files

| File | Purpose |
| --- | --- |
| `final_project_CUI Xiang_CAO Yunliang.c` | Final graphical version |
| `terminal_test.c` | Terminal-only test version, without MLV |
| `1.c` | Early draft |
| `Project_final_de_programmation_C_CUI Xiang_CAO Yunliang.pdf` | Project report (French) |
