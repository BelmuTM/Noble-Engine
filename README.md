<p align="left">
  <img src="resources/icon_large.png" width="128" alt="Noble Engine logo">
</p>

# Noble Engine

Noble Engine is a rendering engine aimed at modern hardware written in C++ with the [Vulkan API](https://www.vulkan.org/).
Currently under active development, it provides support to desktop platforms such as Windows and Linux.
Being able to manipulate 3D environments, render them and use them at our advantage is our goal with this project.

### Contact

Belmu is the main developer and sole contributor of the project.

[![Support me on Patreon](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.vercel.app%2Fapi%3Fusername%3DBelmu%26type%3Dpatrons&style=flat)](https://patreon.com/Belmu)
[![Discord](https://img.shields.io/discord/804772139344461834.svg?logo=discord&logoColor=white&logoWidth=20&labelColor=7289DA&label=Discord)](https://discord.gg/jjRrhpkH9e)
[![Twitter](https://img.shields.io/twitter/follow/Belmu_?color=dark&label=Follow&logoColor=dark)](https://twitter.com/Belmu_)

### Contributing

Please contact Belmu to discuss potential contributions to the project.

# Compiling

## Prerequisites

> [!IMPORTANT]
> C++ 20 (or later) and a 64-bit operating system are required.

### Windows System Requirements

- [CMake 3.22 (or later)](https://github.com/Kitware/CMake/releases/)
    * [In the installer] Check the box indicating: "Add CMake to the system `PATH`".
- [Git](http://git-scm.com/download/win)
    * [In the installer] Check the box allowing to use Git from the command line and from 3rd-party software.

### UNIX-Based System Requirements

- [CMake 3.22 (or later)](https://github.com/Kitware/CMake/releases/)

#### Ubuntu / Debian Package Dependencies
```
sudo apt-get install git libx11-xcb-dev libxkbcommon-dev libXi-dev libwayland-dev libxrandr-dev libxcb-randr0-dev libXinerama-dev libXcursor-dev
```

#### Arch-Based System Package Dependencies
```
sudo pacman -S git libx11 libxcb libxkbcommon libxkbcommon-x11 libxi libxrandr libxinerama libxcursor
```

## Building

### 64-bit Windows
```sh
git clone https://github.com/BelmuTM/Noble-Engine.git  
cd Noble-Engine
git submodule update --init --recursive
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug -A x64
cmake --build ./build/ --config Debug
```

### Linux and macOS
```sh
git clone https://github.com/BelmuTM/Noble-Engine.git  
cd Noble-Engine
git submodule update --init --recursive
cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug
cmake --build ./build/ --config Debug
```

# License

> [!NOTE]  
> Noble Engine is licensed under the [GNU General Public License V3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).
> Consider reading the terms before modifying or redistributing this project.
