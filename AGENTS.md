# Repository Guidelines

## Project Structure & Module Organization

DFToolBox provides C++ tools for inspecting and modifying Dreamfall game data. `src/` contains shared archive, configuration, localization, and Shark3D code, command-line entry points, and Qt6 Widgets GUI components. Shared code is built as `libdftoolbox`; `tinygettext` is a separate library. `icons/` holds GUI assets, `utils/` contains Ruby helper scripts, and `doc/` and `mods/` contain supporting documentation and mod content. Root-level `.ini` files provide configuration examples; `df-directory.txt` supplies known archive filenames.

## Build, Test, and Development Commands

- `nix build` builds the default package using the dependencies defined in `flake.nix`.
- `nix build .#dftoolbox-win64` cross-builds the Windows version, `.#dftoolbox-win64-zip` zips it. The first build compiles Qt and takes a while.
- `nix flake check` builds everything and runs the packaging checks.
- `cmake -S . -B build` configures an out-of-source build. Install CMake 3.15+ and Qt 6.2+ (Widgets, OpenGL, OpenGLWidgets, Multimedia) first; `nix develop` provides them, along with `dftoolbox-build`, `dftoolbox-run` and `dftoolbox-run-gdb`.
- `cmake --build build -j` builds the libraries, GUI applications, and command-line tools.
- `./build/dftoolbox --data-dir .` launches the GUI with repository assets; run from the repository root.
- `./build/dftool-shark3d path/to/file.sdr > /tmp/file.txt` exercises Shark3D decompilation.
- `./build/dftool-model check path/to/res` reads every mesh of every bundle; `location` does the same for scenes.

`SConstruct` and `Makefile` are legacy build paths. Prefer CMake for new build changes.

The version is only written in `VERSION`. Packaging, directories and command line options follow gamepkgstd, see "Packaging" in `README.md` for what does not apply. Platform-specific code stays in `src/system.cpp`.

## Coding Style & Naming Conventions

Match surrounding C++ code: two-space indentation, braces on separate lines, and `.cpp`/`.hpp` source pairs. Use descriptive snake_case filenames and functions, and preserve existing class naming within each module. Keep established license headers. No repository-wide formatter or lint configuration is provided; avoid unrelated reformatting. Register new compiled sources and executables in `CMakeLists.txt`.

## Testing Guidelines

`ctest --test-dir build` runs the few unit tests, there is no coverage threshold. Build affected targets and manually exercise changed behavior on disposable game-data copies. For converter changes, check decompilation and recompilation, including malformed inputs; preserve the trailing `$` in Shark3D text. Record commands, input characteristics, and results in the pull request. Never commit proprietary game archives or extracted assets.

## Related Projects

For 3D model extraction and format references, consult [DreamfallTLJViewer](https://github.com/uchr/DreamfallTLJViewer), which exports meshes and some textures to FBX, and [DreamView](https://github.com/illusionyy/DreamView), which supports mesh export and import.

## Commit & Pull Request Guidelines

Recent commits use short, imperative subjects such as “Fix code not compiling on newer GCC” and “Add Nix flake build files.” Follow that style and keep commits focused. Pull requests should explain the problem, resulting behavior, and validation performed. Link related issues when applicable and include screenshots for visible GUI changes. Mention any changes to configuration or archive compatibility.
