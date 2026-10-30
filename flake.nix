# SPDX-FileCopyrightText: 2026 Ingo Ruhnke <grumbel@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later
{
  description = "Inspect data files of the game Dreamfall";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;

        versionBase = lib.fileContents ./VERSION;
        gitRev = self.shortRev or self.dirtyShortRev or "dirty";
        version =
          if lib.hasInfix "-dev" versionBase
          then "${versionBase}.${toString (self.revCount or 0)}+g${gitRev}"
          else versionBase;

        src = lib.cleanSource ./.;

        qtPluginPath = lib.makeSearchPath "lib/qt-6/plugins" [
          pkgs.qt6.qtbase
          pkgs.qt6.qtmultimedia
        ];

        dftoolbox = pkgs.stdenv.mkDerivation {
          pname = "dftoolbox";
          inherit version src;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            qt6.wrapQtAppsHook
          ];

          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtmultimedia
          ];

          cmakeBuildType = "RelWithDebInfo";
          cmakeFlags = [ "-DPROJECT_VERSION_FULL=${version}" ];

          doCheck = true;

          meta = {
            description = "Inspect and modify the data files of the game Dreamfall";
            homepage = "https://gitlab.com/DFModding/dftoolbox";
            license = lib.licenses.gpl3Plus;
            mainProgram = "dftoolbox";
          };
        };

        # Only x86_64-linux can cross-build these (FLAKE-4), win32 isn't
        # offered because Qt 6 doesn't support it
        windows = lib.optionalAttrs (system == "x86_64-linux")
          (import ./nix/windows.nix { inherit nixpkgs pkgs system version src; });

        packages = {
          default = dftoolbox;
          inherit dftoolbox;
        } // windows;

        apps = rec {
          default = dftoolbox-app;
          dftoolbox = dftoolbox-app;
        };
        dftoolbox-app = flake-utils.lib.mkApp {
          drv = dftoolbox;
          exePath = "/bin/dftoolbox";
        };

        # Sphinx produces no man page in this nixpkgs, which fails the build
        reuse = pkgs.python3Packages.reuse.overridePythonAttrs (old: {
          outputs = lib.remove "man" old.outputs;
          sphinxBuilders = lib.remove "man" old.sphinxBuilders;
        });

        check = name: nativeBuildInputs: script:
          pkgs.runCommand "dftoolbox-check-${name}" { inherit nativeBuildInputs; } ''
            set -euo pipefail
            ${script}
            touch $out
          '';

        appChecks = lib.mapAttrs' (name: app:
          lib.nameValuePair "app-${name}" (check "app-${name}" [ ] ''
            test -x "${app.program}"
          '')) apps;

        devCommand = name: text: pkgs.writeShellScriptBin name ''
          set -euo pipefail
          if [ -z "''${DFTOOLBOX_SOURCE:-}" ]; then
            echo "enter the shell with: nix develop" >&2
            exit 1
          fi
          ${text}
        '';

        dftoolbox-configure = devCommand "dftoolbox-configure" ''
          cmake -S "$DFTOOLBOX_SOURCE" -B "$DFTOOLBOX_BUILD_DIR" -G Ninja \
            -DCMAKE_BUILD_TYPE="''${CMAKE_BUILD_TYPE:-Debug}" \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
          if [ ! -e "$DFTOOLBOX_SOURCE/build" ] || [ -L "$DFTOOLBOX_SOURCE/build" ]; then
            ln -sfn "$DFTOOLBOX_BUILD_DIR" "$DFTOOLBOX_SOURCE/build"
          fi
        '';

        dftoolbox-build = devCommand "dftoolbox-build" ''
          cache="$DFTOOLBOX_BUILD_DIR/CMakeCache.txt"
          home=""
          if [ -f "$cache" ]; then
            home=$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$cache")
          fi
          if [ "$home" != "$DFTOOLBOX_SOURCE" ] || [ ! -d "$home" ]; then
            rm -f "$cache"
            dftoolbox-configure
          fi
          cmake --build "$DFTOOLBOX_BUILD_DIR"
        '';

        dftoolbox-run = devCommand "dftoolbox-run" ''
          dftoolbox-build
          export QT_PLUGIN_PATH="${qtPluginPath}"
          exec "$DFTOOLBOX_BUILD_DIR/dftoolbox" --data-dir "$DFTOOLBOX_SOURCE" "$@"
        '';

        dftoolbox-run-gdb = devCommand "dftoolbox-run-gdb" ''
          dftoolbox-build
          export QT_PLUGIN_PATH="${qtPluginPath}"
          exec gdb -q \
            -ex "set pagination off" \
            -ex "set confirm off" \
            -ex "set debuginfod enabled off" \
            -ex run \
            -ex 'python
          try:
            if int(gdb.parse_and_eval("$_exitcode")) == 0:
              gdb.execute("quit")
          except Exception:
            pass
          ' \
            --args "$DFTOOLBOX_BUILD_DIR/dftoolbox" --data-dir "$DFTOOLBOX_SOURCE" "$@"
        '';
      in {
        inherit packages apps;

        devShells.default = pkgs.mkShell {
          inputsFrom = [ dftoolbox ];
          packages = with pkgs; [
            cmake
            ninja
            gdb
            ccache
            dftoolbox-configure
            dftoolbox-build
            dftoolbox-run
            dftoolbox-run-gdb
          ];
          shellHook = ''
            export DFTOOLBOX_SOURCE="$PWD"
            export DFTOOLBOX_BUILD_DIR="''${DFTOOLBOX_BUILD_DIR:-/tmp/dftoolbox-build}"
          '';
        };

        checks = packages // appChecks // {
          version = check "version" [ ] ''
            test "$(${dftoolbox}/bin/dftoolbox --version)" = "dftoolbox ${version}"
          '';

          help = check "help" [ ] ''
            ${dftoolbox}/bin/dftoolbox --help
          '';

          layout = check "layout" [ ] ''
            cd ${dftoolbox}
            for file in \
              bin/dftoolbox \
              bin/dfmodtool \
              share/dftoolbox/df-directory.txt \
              share/dftoolbox/icons/dftoolbox.png \
              share/applications/dftoolbox.desktop \
              share/metainfo/dftoolbox.metainfo.xml \
              share/icons/hicolor/{16x16,32x32,48x48,64x64,128x128,256x256}/apps/dftoolbox.png \
              share/man/man1/dftoolbox.1.gz
            do
              test -e "$file" || { echo "missing: $file"; exit 1; }
            done
          '';

          integration = check "integration" (with pkgs; [ desktop-file-utils appstream ]) ''
            desktop-file-validate ${dftoolbox}/share/applications/dftoolbox.desktop
            # The non-reverse-DNS ID is deliberate (INT-1)
            appstreamcli validate --no-net ${dftoolbox}/share/metainfo/dftoolbox.metainfo.xml \
              | tee result || true
            if grep -E '^(E|W):' result | grep -v cid-desktopapp-is-not-rdns; then
              exit 1
            fi
          '';

          man = check "man" [ pkgs.mandoc ] ''
            mandoc -T lint -W warning ${./man/dftoolbox.1}
          '';

          reuse = check "reuse" [ reuse ] ''
            reuse --root ${src} lint
          '';
        } // lib.optionalAttrs (system == "x86_64-linux") {
          windows-icon = check "windows-icon" [ pkgs.icoutils ] ''
            for exe in dftoolbox dfmodtool; do
              wrestool -l ${windows.dftoolbox-win64}/$exe.exe | grep -q 'type=14'
              wrestool -l ${windows.dftoolbox-win64}/$exe.exe | grep -q 'type=16'
            done
          '';
        };
      }
    );
}
