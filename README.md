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

### Linux Package Dependencies

<details>
<summary>Ubuntu / Debian Package Dependencies</summary>

```
sudo apt-get update
sudo apt-get install -y \
    cmake \
    git \
    libx11-xcb-dev \
    libxkbcommon-dev \
    libXi-dev \
    libwayland-dev \
    libxrandr-dev \
    libxcb-randr0-dev \
    libXinerama-dev \
    libXcursor-dev
```
</details>

<details>
<summary>Arch Linux dependencies</summary>

```bash
sudo pacman -S \
    cmake \
    git \
    libx11 \
    libxcb \
    libxkbcommon \
    libxkbcommon-x11 \
    libxi \
    libxrandr \
    libxinerama \
    libxcursor
```
</details>

## Building

### Quick start (auto-detect compiler)

```sh
git clone https://github.com/BelmuTM/Noble-Engine.git  
cd Noble-Engine
git submodule update --init --recursive
cmake --preset dev

cmake --build --preset debug
# OR
cmake --build --preset release
```

### Platform-specific presets

| Platform | Compiler | Configure preset | Build preset          | Configuration |
|----------|----------|------------------|-----------------------|---------------|
| Linux    | GCC      | linux-gcc        | linux-gcc-debug       | Debug         |
|          |          |                  | linux-gcc-release     | Release       |
| Linux    | Clang    | linux-clang      | linux-clang-debug     | Debug         |
|          |          |                  | linux-clang-release   | Release       |
| Windows  | MSVC     | windows-msvc     | windows-msvc-debug    | Debug         |
|          |          |                  | windows-msvc-release  | Release       |
| Windows  | Clang-cl | windows-clang    | windows-clang-debug   | Debug         |
|          |          |                  | windows-clang-release | Release       |

# License

> [!NOTE]  
> Noble Engine is licensed under the [GNU General Public License V3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).
> Consider reading the terms before modifying or redistributing this project.

<div align="center">
  Made with ❤️
</div>
