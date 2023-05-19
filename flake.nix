{
  description = "Inspect data files of the game Dreamfall";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
       in {
         packages = rec {
           default = dftoolbox;

           dftoolbox = pkgs.stdenv.mkDerivation {
             name = "dftoolbox";
             src = nixpkgs.lib.cleanSource ./.;
             cmakeFlags = [];

             nativeBuildInputs = with pkgs; [
               cmake
               pkgconfig
             ];

             buildInputs = with pkgs; [
               fox_1_6
               SDL
               SDL_mixer
             ];
           };
        };
       }
    );
}
