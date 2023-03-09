# tessalatrix

[![CMake](https://github.com/ahnlak/tessalatrix/actions/workflows/cmake.yml/badge.svg)](https://github.com/ahnlak/tessalatrix/actions/workflows/cmake.yml)

A flexible cross platform version of Tetris, using C and SDL.

This is primarily being developed to give me a way to explore the modern
state of SDL, with a view to building something a little more ... original
in the future.

It draws a lot of inspiration from [Tetris for Terminals](https://github.com/MikeTaylor/tt)
which I spent *way* too much of my student days playing, in terms of pacing,
scoring and the extended tile sets.

Ultimately, I aim to be able to target multiple platforms (Linux & Windows
definitely, Web and even MacOS ideally) with a common code base. At the time
of writing, automated builds target Linux, Windows and Web.

As always, this is all released under the MIT License.

Share And Enjoy

## Building

The easiest way to get a playable copy of the game is to head to the 
[Releases](https://github.com/ahnlak/tessalatrix/releases) page - each release
should have automatically built zip / tar files for all supported targets.
Simply grab the file and unzip it into a suitable directory, and run the
`tessalatrix(.exe)` file within.

If you want to build from source, refer to 
[BUILDING.md](https://github.com/ahnlak/tessalatrix/BUILDING.md) but be
aware that this only talks in detail about Linux-based builds because that's
my primary development environment.