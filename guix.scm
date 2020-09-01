;; DFToolBox
;; Copyright (C) 2019 Ingo Ruhnke <grumbel@gmail.com>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(set! %load-path
  (cons* "/ipfs/QmNbci3WpEWumBVvfbqsKhKnJTtdmkyVwYtSaPr5jVZo2m/guix-cocfree_0.0.0-56-g3a32b64"
         %load-path))

(use-modules (guix build-system cmake)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (gnu packages graphics)
             (gnu packages sdl)
             (gnu packages pkg-config)
             (guix-cocfree utils))

(define %source-dir (dirname (current-filename)))

(define-public dftoolbox
  (package
    (name "dftoolbox")
    (version (version-from-source %source-dir))
    (source (source-from-source %source-dir #:version version))
    (build-system cmake-build-system)
    (arguments
     `(#:tests? #f))
    (native-inputs
     `(("pkg-config" ,pkg-config)))
    (inputs
     `(("fox" ,fox)
       ("sdl" ,sdl)
       ("sdl-mixer" ,sdl-mixer)))
    (synopsis "Inspect data files of the game Dreamfall")
    (description "DFToolBox is a set of tools that allow you to view and
manipulate the data files (.pak) of the Funcom game Dreamfall.
Currently supported features:

@itemize
@item extract .pak files with (some) filenames
@item replace files inside a .pak
@item convert localisation.dat to .po files for translation
@item convert a .po file back into localisation.dat
@item convert .sri and other shark3d_snake files into a textual representation
@item convert text back to shark3d_snake files
@end itemize")
    (home-page "https://gitlab.com/dftoolbox/dftoolbox")
    (license license:gpl3+)))

dftoolbox

;; EOF ;;
