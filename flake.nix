{
  description = "Inspect data files of the game Dreamfall";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
       in rec {
         packages = flake-utils.lib.flattenTree rec {
           dftoolbox = pkgs.stdenv.mkDerivation {
             name = "dftoolbox";
             src = nixpkgs.lib.cleanSource ./.;
             cmakeFlags = [];
             nativeBuildInputs = [
               pkgs.cmake
               pkgs.pkgconfig
             ];
             buildInputs = [
               pkgs.fox_1_6
               pkgs.SDL
               pkgs.SDL_mixer
             ];
           };
        };
        defaultPackage = packages.dftoolbox;
      });
}
