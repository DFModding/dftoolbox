DFToolBox
=========

DFToolBox is a set of tools that allow you to view and manipulate the
data files (.pak) of the Funcom game Dreamfall. Currently supported
features:

- extract .pak files with (some) filenames
- replace files inside a .pak
- convert localisation.dat to .po files for translation
- convert a .po file back into localisation.dat
- convert .sri and other shark3d_snake files into a textual representation
- convert text back to shark3d_snake files
- view locations, characters and objects in 3D

Usage
-----

All those tools in here are command line tools, meaning they don't
have a GUI and for some you might have to use cmd.exe, however many
are also usable from the GUI by just drag&droping files on the
relevant .exe

### extract

This is a basic extractor for .pak files, drag any files you want to
extract on the extractor.exe and the extractor will extract the files
to dreamfall/ in the same directory as the extract.exe.

### insert

This tool will allow you to insert files into Dreamfall, for that
there exists a directory dreamfall-mods/ into which you should place
you modifications, in `$XDG_DATA_HOME/dftoolbox/` on Linux and
`%LOCALAPPDATA%\dftoolbox\data\` on Windows. A dreamfall-mods/ in the
current directory, or next to the .exe on Windows, is still used if
that one is missing. The insert tool will, once started, scan that
directory for files and insert them into the .paks of Dreamfall. The
.paks will be automatically backed up to .pak.orig, so you can rename
them back if you want to get rid of you modifications.

### dialog

A tool to extract and generate dialog that is stored in, use via
cmd.exe:

    data/generated/config/universe/localization.dat

### shark3d

shark3d is a compiler/decompiler for shark3d_snake_binary files (.cdr,
.sgr, ...). To decompile use:

    shark3d art/shaders/generic_water.sdr > generic_water.txt

To compile use:

    shark3d --compile generic_water.txt > generic_water.sdr

### Browsing

The tree of the `dftoolbox` GUI lists the files under "all paks", per
pak under "by pak", and sorted by what they are under "by type": 3d
models, animations, localization, music, scripts, shaders, sound
effects, textures and voice, the latter split by language. Files that
only support other files, like `.lip`, are not listed there.

### 3D preview

The `dftoolbox` GUI shows `.bun` and `.sir` files in an OpenGL 3.3
view. Selecting a bundle displays its location, the tree next to the
view lists the models in it. Drag with the left mouse button to
rotate, with the right one to move, use the wheel to zoom and
W/A/S/D/Q/E to fly around. "Source" and "3D" switch a `.sir` between
the view and its text.

Animations play on their own, "Play" pauses them. Characters get a
list of the animations in their directory that fit their skeleton,
starting with an idle one if they have it.

Models are shown with the baked lighting of the game, the game's
dynamic lights are not reproduced, so some locations are very dark,
turn off "Lighting" to see those. Normal maps and the game's own
shader programs are not implemented.

`dftool-model` checks the same code from the command line:

    dftool-model list FILE.bun
    dftool-model check PAKDIR
    dftool-model location PAKDIR [NAME]...
    dftool-model sir PAKDIR FILE.sir [PAK]

The format is described in `doc/bundle-format.txt`.

Status
------

Nearly every file in the game has been identified. What is left is
mostly lip sync, lighting and IK data, plus code that hasn't been
written yet. File counts are from the 103,223 files in the .paks of
the PC version, about 350 of those have no known filename.

### What works

| Format | Files | State |
|---|---|---|
| `.pak` | 88 | Read and write, some filenames can't be recovered |
| Shark3D binary (`.cdr .sgr .sdr .sir .skr .ikr .rgr .spr .str .svr`) | ~25,000 | Decompile and compile, the content is self-describing |
| `.bun` | 70 | Geometry, textures and stages are decoded and shown in the 3D preview |
| DDS textures, often named `.png` or `.tga` | ~9,850 | Decoded |
| `.scr` | 3,509 | Plain text D3D shader assembly |
| `.mp3`, `.wav` | ~28,000 | Standard formats, can be played |
| `localization.dat` | 1 | Converts to and from `.po` |

### Animation

| Format | Files | State |
|---|---|---|
| `.bpr` (`tljbone`) | ~9,000 | Versions `0` and `2` are played, the 2,846 files with a known name all parse |
| `.skr` | 361 | Bone parents are used by the 3D preview |
| `.spr` | 310 | Keyframed paths of the objects in a location are played |
| Bone data in `.bun` | | Skinning works, all 558 skinned meshes find their skeleton |
| Tween frames in `.bun` | 152 meshes | Played, the time between frames is inferred from DreamView |

The fourth bone weight is taken to be one minus the other three.
Walking and crouching look right with that, but it comes from the
shader and hasn't been seen in the data directly.

### Partly understood

- STFU4 (2,214 files, nearly all normal maps): `dftool-stfu4` is still
  an experiment. The layout looks like a quarter size bump plane plus
  blocks of normal data. DreamfallTLJViewer has a parser for it which
  has yet to be studied.
- `.bun`: a few header fields are unlabelled, nothing that gets drawn
  depends on them.

### Unknown

- `.lip` (22,817 files): one for each voice line, seemingly 4 byte
  records, likely visemes or timing. Neither DreamView nor
  DreamfallTLJViewer read these.
- `.ikr_data` (1,152 files): starts with a version-like `3`, followed
  by floats in what look like fixed size records. Probably IK targets,
  `.rgr` and `.ikr` tell which bones and animations they belong to.
- `art/lights/*.dat`: two counts followed by floats, probably baked
  light data. Decoding those could also help with the dark locations.

### Not examined yet

- about 300 of the 373 `.dat` files start with the text `art/`, so
  they aren't the light format
- 904 `.bpr` files are Shark3D instead of `tljbone`, these and two
  `tljbone1` files are not played
- 85 `.smr` files in Shark3D format
  (`shark3d_std_model_runtime_3x2x1`) that look like GUI models, the
  3D preview can't show those

### What can't be done yet

- animations are only found when their filename is known, so those
  missing from `df-directory.txt` aren't offered in the preview
- location previews don't show `anim_*` models such as doors, the
  game starts those when something happens, and characters aren't
  placed in them
- animations play in a loop, nothing of the game's scripts, blending
  or IK is reproduced
- no dynamic lights, normal maps or original shader programs
- no way to export or import models, DreamfallTLJViewer and DreamView
  can be used for that
- the 3D preview is read only, models can't be changed from it
- lip sync data can't be read or generated

Credits
-------

The 3D preview would not exist without the reverse engineering done
by these projects, which were used as a reference for the bundle
format and the handling of scenes and shaders:

* [DreamView](https://github.com/illusionyy/DreamView) by Tobias Pfaff
  (GPLv2+), a viewer and exporter for Dreamfall models
* [DreamfallTLJViewer](https://github.com/uchr/DreamfallTLJViewer) by
  Anton Tarasov (MIT), which exports meshes and textures to FBX

Issues
------

- .pak files only store a hash-map with the filenames, not a normal
  index, so its impossible to figure out the correct filename for many
  files, the extractor will atemp to guess the filename, however
  filenames will thus often come out wrong. The df-directory.txt file
  is used to minimize the throuble, by providing a predefined, but
  incomplete, list of filenames

- the shark3d text parser is a bit buggy, so don't remove the '$' at
  the end of converted shark3d files

Contact
-------

The latest version of dftoolbox can be found at:

* http://clanlib.org/~grumbel/

Question and commenst should either go to:

* EMail: grumbel@gmail.com

or the pak extraction thread in the Dreamfall forum:

* http://forums.longestjourney.com/showthread.php?t=2352

or use the IRC server `irc.rizon.net`, channel `#ragnar`

License
-------

GPL-3.0-or-later, see `COPYING`. Earlier versions were GPL-2.0-or-later.
The mods in `mods/` are by skribb and are not covered by this, see
`LICENSES/LicenseRef-Dreamfall-Mods.txt`. They are packaged anyway to
keep them available.

Building
--------

    nix build                        # Linux
    nix build .#dftoolbox-win64      # Windows, unpacked
    nix build .#dftoolbox-win64-zip  # Windows, zipped
    nix flake check                  # everything, plus the packaging checks

The first Windows build compiles Qt, which takes a while. `nix develop`
provides `dftoolbox-build`, `dftoolbox-run` and `dftoolbox-run-gdb`.

Packaging
---------

Builds, directories and command line options follow gamepkgstd. As
DFToolBox is a tool rather than a game, these rules are deviated from
or skipped:

| Rules | Why |
|---|---|
| KEY-*, OPT-5, OPT-8, STACK-3, STACK-4, STACK-5 | Gamepad, game keys, game settings and the 640x480 minimum are about games, the keyboard shortcuts are Qt's |
| OPT-3 `--fullscreen`, `--windowed`, `--geometry`, `--mute`, `--state-dir` | No fullscreen mode, sound only plays on request, no state: savegames are the game's own |
| STACK-2, INT-3, INT-9, SDL3-* | Qt instead of SDL, as STACK-6 asks for: Qt Multimedia plays the sounds, `setDesktopFileName()` sets the window IDs |
| DIR-1, INT-7 man section, categories | The man page is in section 1, and the .desktop file is not in `Game`, since this is not a game |
| FLAKE-1 `dftoolbox-win32` | Qt 6 has no 32-bit Windows support |
| FLAKE-14 | The project is hosted on GitLab and Radicle, not GitHub. `nix flake check` compiles Qt, which also doesn't fit shared CI runners |
| Android, PortMaster, web | Not targets |

The DIR-3 search means that a checkout has to be told where its data
is, `dftoolbox-run` does that, by hand it is `--data-dir .`.
`--datadir`, `--configdir` and `--cachedir` still work. Windows moved
from `%APPDATA%\dftoolbox\` to its `config\` subdirectory, existing files
in the old location are still read.
