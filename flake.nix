{
  description = "Inspect data files of the game Dreamfall";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";
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
             pname = "dftoolbox";
             version = "0.0.0";

             src = ./.;

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
