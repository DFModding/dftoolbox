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
       in {
         packages = rec {
           default = dftoolbox;

           dftoolbox = pkgs.stdenv.mkDerivation {
             pname = "dftoolbox";
             version = "0.1.0";

             src = ./.;

             nativeBuildInputs = with pkgs; [
               cmake
               pkg-config
               qt6.wrapQtAppsHook
             ];

             buildInputs = with pkgs; [
               qt6.qtbase
               SDL
               SDL_mixer
             ];
           };
        };

         devShells.default = pkgs.mkShell {
           inputsFrom = [ self.packages.${system}.dftoolbox ];
         };
       }
    );
}
