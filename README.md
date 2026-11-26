<div align="center">

<img src="./assets/logo.png" alt="Logo" width="400">

# MAPD Visualizer

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
[![ci](https://github.com/Kei18/mapf-visualizer/actions/workflows/ci.yml/badge.svg)](https://github.com/Kei18/mapf-visualizer/actions/workflows/ci.yml)

Native viewer for multi-agent pickup and delivery (MAPD) and multi-agent pathfinding (MAPF) recordings.

Based on [openFrameworks](https://openframeworks.cc/), written in C++14.

</div>

## Demo

![room-32-32-4](./assets/demo_room.gif)

![tunnel, planning with four agents](./assets/demo_tunnel.gif)

![ost003d, planning with 1000 agents](./assets/demo_ost003d.gif)

## Build from Terminal

### macOS

Install the Xcode Command Line Tools with `xcode-select --install` if needed. From the repository root, download the [openFrameworks 0.12.0 macOS release](https://github.com/openframeworks/openFrameworks/releases/tag/0.12.0) and build against its bundled libraries.

```sh
mkdir -p build
curl -fL https://github.com/openframeworks/openFrameworks/releases/download/0.12.0/of_v0.12.0_osx_release.zip -o build/openframeworks-osx.zip
ditto -x -k build/openframeworks-osx.zip build
make -j4 OF_ROOT="$PWD/build/of_v0.12.0_osx_release"
```

### Ubuntu 24.04

Unpack the openFrameworks 0.12.0 Linux release into `third_party/openFrameworks`, then install its system dependencies and build.

```sh
wget -O third_party/openFrameworks.tar.gz https://github.com/openframeworks/openFrameworks/releases/download/0.12.0/of_v0.12.0_linux64gcc6_release.tar.gz
tar -xzf third_party/openFrameworks.tar.gz -C third_party --strip-components=1 --one-top-level=openFrameworks
sudo third_party/openFrameworks/scripts/linux/ubuntu/install_dependencies.sh
sudo third_party/openFrameworks/scripts/linux/ubuntu/install_codecs.sh
make -j4
```

On other Linux distributions, see [install_linux.sh](install_linux.sh).

## Run from Terminal

The launcher opens a native window. Run the bundled three-agent demo or pass your own files.

```sh
./run-visualizer --demo
./run-visualizer path/to/map.emap path/to/paths.txt path/to/tasks.txt
```

Tasks are optional. A two-file run uses the final path poses as goals and still supports orientation.

```sh
./run-visualizer assets/2x2.map assets/demo_2x2.txt
```

![](./assets/orientation.gif)

## MAPD input formats

- **Map.** An extended `.emap` adds a `regions` header and a second grid with the same dimensions as the Moving AI terrain grid. Each walkable cell has a region digit from 0 through 9. The viewer shades each region and outlines boundaries.
- **Ports.** A pair of equal arrows (`<`, `>`, `^`, or `v`) straddles a region boundary. The arrow pointing toward its peer is the outlet. The receiving cell is the inlet. Both cells are traversable and are labeled **OUT** and **IN**.
- **Paths.** Each row starts with a consecutive timestep beginning at zero, followed by poses in agent ID order. A pose is `(x,y)` or `(x,y,orientation)`, where `(0,0)` is the upper-left cell. For example, `0:(5,1),(6,4),(2,4),`.
- **Tasks.** The first line is the agent count. Each subsequent agent row contains queued entries in the form `(x,y),completion_time,;`. Use `-1` for pending tasks. An optional `Total completed tasks: N` footer is checked against the records.

At timestep `t`, a task with completion time `t` has finished and the next goal becomes active. The first pending goal stays active for the rest of the recording. An agent with no remaining tasks has no goal marker. The sidebar shows the active goal for each agent. The recorded queue is treated as available from timestep zero and describes global goals.

## Playback controls

- `p` plays or pauses, `l` toggles looping, and `r` resets the timeline.
- The timeline slider seeks directly. With a tasks file, the left and right arrows pause and step one timestep.
- `g` toggles goal markers, `v` cycles through goal vectors and path traces, and `f` toggles agent and cell IDs.
- `[` and `]` page through the goal list when it exceeds the panel. Drag to pan, use `i` or `o` to zoom, and press `Esc` to close.

In the demo, agents 0 and 2 switch goals at timestep 7. Agent 1 switches at timestep 9.

## Check the recordings

These checks run without openFrameworks.

```sh
mkdir -p build
c++ -std=c++14 -Iinclude src/graph.cpp src/recording.cpp tests/check_recordings.cpp -o build/check-recordings
./build/check-recordings
```

## Notes

- The grid maps in `assets/` are from [MAPF benchmarks](https://movingai.com/benchmarks/mapf.html)
- The font in `visualizer/bin/data` is from [Google Fonts](https://fonts.google.com/)
- This repo is forked from a series of my previous studies such as
  [pibt2](https://kei18.github.io/pibt2),
  [mapf-IR](https://kei18.github.io/mapf-IR/),
  [otimapp](https://kei18.github.io/otimapp/), etc.
  This repo is compatible with
  [lacam](https://kei18.github.io/lacam/),
  [tswap](https://kei18.github.io/tswap/),
  [pypibt](https://github.com/Kei18/pypibt), etc.
- Install [pre-commit](https://pre-commit.com/):
```sh
pre-commit install
```
- I'm happy if this repo helps somebody. Please use "Cite this repository" on this page for citation.

## Licence

This software is released under the MIT License, see [LICENSE.txt](LICENCE.txt).

## Maintainer

[Keisuke Okumura](https://kei18.github.io)
