# Repository Guidelines

## Project Structure & Module Organization

DFToolBox provides C++ tools for inspecting and modifying Dreamfall game data. `src/` contains shared archive, configuration, localization, and Shark3D code, command-line entry points, and FOX GUI components. Shared code is built as `libdftoolbox`; `tinygettext` is a separate library. `icons/` holds GUI assets, `utils/` contains Ruby helper scripts, and `doc/` and `mods/` contain supporting documentation and mod content. Root-level `.ini` files provide configuration examples; `df-directory.txt` supplies known archive filenames.

## Build, Test, and Development Commands

- `nix build` builds the default package using the dependencies defined in `flake.nix`.
- `cmake -S . -B build` configures an out-of-source build. Install CMake 3.15+, pkg-config, Qt 6.2+ (Widgets), FOX 1.6 (until the Qt6 port is complete), SDL 1.2, and SDL_mixer first; pkg-config must resolve `fox`, `sdl`, and `SDL_mixer`, and CMake must find `Qt6`. `nix develop` provides all of them.
- `cmake --build build -j` builds the libraries, GUI applications, and command-line tools.
- `./build/dftoolbox --datadir .` launches the GUI with repository assets; run from the repository root.
- `./build/dftool-shark3d path/to/file.sdr > /tmp/file.txt` exercises Shark3D decompilation.

`SConstruct` and `Makefile` are legacy build paths. Prefer CMake for new build changes.

## Coding Style & Naming Conventions

Match surrounding C++ code: two-space indentation, braces on separate lines, and `.cpp`/`.hpp` source pairs. Use descriptive snake_case filenames and functions, and preserve existing class naming within each module. Keep established license headers. No repository-wide formatter or lint configuration is provided; avoid unrelated reformatting. Register new compiled sources and executables in `CMakeLists.txt`.

## Testing Guidelines

No automated test framework or coverage threshold is configured. Build affected targets and manually exercise changed behavior on disposable game-data copies. For converter changes, check decompilation and recompilation, including malformed inputs; preserve the trailing `$` in Shark3D text. Record commands, input characteristics, and results in the pull request. Never commit proprietary game archives or extracted assets.

## Related Projects

For 3D model extraction and format references, consult [DreamfallTLJViewer](https://github.com/uchr/DreamfallTLJViewer), which exports meshes and some textures to FBX, and [DreamView](https://github.com/illusionyy/DreamView), which supports mesh export and import.

## Commit & Pull Request Guidelines

Recent commits use short, imperative subjects such as “Fix code not compiling on newer GCC” and “Add Nix flake build files.” Follow that style and keep commits focused. Pull requests should explain the problem, resulting behavior, and validation performed. Link related issues when applicable and include screenshots for visible GUI changes. Mention any changes to configuration or archive compatibility.
