# SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

# Windows build (mingw-w64 cross, win64 only: Qt 6 has no 32-bit
# Windows support).
#
# Output (gamepkgstd DIR-2, FLAKE-1/3): the .exe files, their DLLs, the
# Qt plugins and data/ side by side; the zip has one top-level directory
# dftoolbox-<version>-win64/.
#
#   nix build .#dftoolbox-win64
#   nix build .#dftoolbox-win64-zip

{ nixpkgs
, pkgs            # native, for tools
, system
, version
, src
}:

let
  lib = pkgs.lib;

  crossPkgs = import nixpkgs {
    inherit system;
    crossSystem = { config = "x86_64-w64-mingw32"; };
    overlays = [
      (final: prev: if !prev.stdenv.hostPlatform.isMinGW then { } else {
        # Its python3 is only for the tests, and is broken on mingw
        graphite2 = prev.graphite2.overrideAttrs (old: {
          nativeBuildInputs = builtins.filter
            (dep: builtins.match "python3.*" (dep.name or "") == null)
            (old.nativeBuildInputs or [ ]);
          doCheck = false;
        });

        # Qt uses opengl32.dll on Windows, libglvnd doesn't cross-build
        qt6 = prev.qt6.overrideScope (qtFinal: qtPrev: {
          qtbase = qtPrev.qtbase.override { libGL = null; };

          # Only needs zlib. Its other inputs are for image plugins that
          # aren't part of it, and don't cross-build.
          qtsvg = qtPrev.qtsvg.overrideAttrs (old: {
            buildInputs = [ final.zlib ];
          });

          # mingw's headers are lowercase and Linux is case sensitive
          qtmultimedia = qtPrev.qtmultimedia.overrideAttrs (old: {
            postPatch = (old.postPatch or "") + ''
              substituteInPlace src/plugins/multimedia/ffmpeg/ffmpeg_pch.h \
                --replace-fail "<D3d11.h>" "<d3d11.h>"
            '';
          });
        });

        # The default FFmpeg pulls in libraries that don't cross-build
        # and is marked as broken for that. Qt only needs the codecs and
        # formats of FFmpeg itself.
        ffmpeg = (prev.ffmpeg.override {
          withHeadlessDeps = false;
          withSmallDeps = false;
          withFullDeps = false;
          withNetwork = false;
          # Disabled along with the above, these are what Qt needs
          buildAvcodec = true;
          buildAvformat = true;
          buildAvutil = true;
          buildSwresample = true;
          buildSwscale = true;
          buildFfmpeg = false;
          buildFfplay = false;
          buildFfprobe = false;
          withDocumentation = false;
          withHtmlDoc = false;
          withManPages = false;
          withPodDoc = false;
          withTxtDoc = false;
        }).overrideAttrs (old: { meta = old.meta // { broken = false; }; });
      })
    ];
  };

  qt = crossPkgs.qt6;

  build = crossPkgs.stdenv.mkDerivation {
    pname = "dftoolbox-win64-build";
    inherit version src;

    nativeBuildInputs = [ pkgs.cmake pkgs.ninja ];
    buildInputs = [ qt.qtbase qt.qtmultimedia ];

    cmakeFlags = [
      "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
      "-DPROJECT_VERSION_FULL=${version}"
      "-DBUILD_TESTING=OFF"
      "-DQT_HOST_PATH=${pkgs.qt6.qtbase}"
    ];

    dontWrapQtApps = true;
  };

  closure = pkgs.closureInfo {
    rootPaths = [
      build
      qt.qtbase
      qt.qtmultimedia
      crossPkgs.stdenv.cc.cc.lib
      crossPkgs.ffmpeg.out
      (lib.getBin crossPkgs.libjpeg)
    ];
  };

  folder = pkgs.runCommand "dftoolbox-win64-${version}" {
    nativeBuildInputs = [ crossPkgs.stdenv.cc.bintools ];
  } ''
    mkdir -p $out
    cp -r --no-preserve=mode ${build}/bin/*.exe ${build}/data $out/

    for plugin in \
      platforms/qwindows \
      styles/qmodernwindowsstyle \
      imageformats/qgif \
      imageformats/qico \
      imageformats/qjpeg
    do
      mkdir -p "$out/$(dirname $plugin)"
      cp --no-preserve=mode ${qt.qtbase}/lib/qt-6/plugins/$plugin.dll "$out/$plugin.dll"
    done
    mkdir -p $out/multimedia
    cp --no-preserve=mode ${qt.qtmultimedia}/lib/qt-6/plugins/multimedia/*.dll $out/multimedia/

    # Copies every DLL that the .exe files and plugins need from the
    # closure. Names that aren't found are the ones of Windows itself.
    declare -A available
    while read -r path; do
      for dll in "$path"/bin/*.dll "$path"/lib/*.dll "$path"/*/lib*/*.dll; do
        if [ -e "$dll" ]; then
          name=$(basename "$dll")
          available[''${name,,}]=$dll
        fi
      done
    done < ${closure}/store-paths

    todo=$(find $out -name '*.exe' -o -name '*.dll')
    while [ -n "$todo" ]; do
      next=""
      for file in $todo; do
        for name in $(x86_64-w64-mingw32-objdump -p "$file" | sed -n 's/^\s*DLL Name: //p'); do
          dll=''${available[''${name,,}]:-}
          if [ -n "$dll" ] && [ ! -e "$out/$(basename "$dll")" ]; then
            cp --no-preserve=mode "$dll" $out/
            next="$next $out/$(basename "$dll")"
          fi
        done
      done
      todo=$next
    done

    cp ${../COPYING} $out/COPYING.txt
    cat > $out/README.txt <<'EOR'
    DFToolBox ${version}
    Inspect and modify the data files of the game Dreamfall.

    Start dftoolbox.exe, or dfmodtool.exe to install mods. The dftool-*.exe
    programs are command line tools.

    Settings: %APPDATA%\dftoolbox\config\
    Portable mode: create a file portable.txt next to dftoolbox.exe.

    Licence: GPL-3.0-or-later (COPYING.txt).
    Source: https://github.com/DFModding/dftoolbox
    EOR
    sed -i 's/^    //; s/$/\r/' $out/README.txt
  '';

  zip = pkgs.runCommand "dftoolbox-${version}-win64.zip" {
    nativeBuildInputs = [ pkgs.zip ];
  } ''
    mkdir -p $out
    work=$(mktemp -d)
    cp -r --no-preserve=mode ${folder} "$work/dftoolbox-${version}-win64"
    cd "$work"
    zip -r "$out/dftoolbox-${version}-win64.zip" "dftoolbox-${version}-win64"
  '';
in {
  dftoolbox-win64 = folder;
  dftoolbox-win64-zip = zip;
}
