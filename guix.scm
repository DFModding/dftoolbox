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

(use-modules (ice-9 popen)
             (ice-9 rdelim)
             (guix build utils)
             (guix build-system gnu)
             (guix build-system scons)
             (guix gexp)
             (guix download)
             (guix git-download)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (gnu packages gcc)
             (gnu packages gl)
             (gnu packages image)
             (gnu packages compression)
             (gnu packages perl)
             (gnu packages fontutils)
             (gnu packages linux)
             (gnu packages sdl)
             (gnu packages pkg-config)
             (gnu packages xorg))

(define %source-dir (dirname (current-filename)))

(define current-commit
  (with-directory-excursion %source-dir
    (let* ((port   (open-input-pipe "git describe --tags"))
           (output (read-line port)))
      (close-pipe port)
      (string-trim-right output #\newline))))

(define-public fox
  (package
   (name "fox")
   (version "1.6.57")
   (source
    (origin
     (method url-fetch)
     (uri (string-append
           "ftp://ftp.fox-toolkit.org/pub/fox-" version ".tar.gz"))
     (sha256
      (base32
       "08w98m6wjadraw1pi13igzagly4b2nfa57kdqdnkjfhgkvg1bvv5"))))
   (build-system gnu-build-system)
   (arguments
     `(#:tests? #f                      ; no check target
       #:make-flags
       (list
        ;; Xft.h #includes <ft2build.h> (without ‘freetype2/’).  The sxiv
        ;; Makefile works around this by hard-coding /usr/include instead.
        (string-append "CPPFLAGS=-I"
                       (assoc-ref %build-inputs "freetype")
                       "/include/freetype2"))))
   (propagated-inputs
    `(("freetype" ,freetype)))
   (inputs
    `(("libjpeg" ,libjpeg)
       ("libpng" ,libpng)
       ("libtiff" ,libtiff)
       ("zlib" ,zlib)
       ("bzip2" ,bzip2)
       ("glu" ,glu)
       ("mesa" ,mesa)
       ("libx11" ,libx11)
       ("libxcursor" ,libxcursor)
       ("libxi" ,libxi)
       ;;("libxinerama" ,libxinerama)
       ;;("libxkbcommon" ,libxkbcommon)
       ;;("libxdamage" ,libxdamage)
       ("libxrandr" ,libxrandr)
       ("libxft" ,libxft)
       ("fontconfig" ,fontconfig)
       ("linux-libre-headers" ,linux-libre-headers)
       ))
   (native-inputs
    `(("perl" ,perl)))
   (synopsis "")
   (description "")
   (home-page "http://fox-toolkit.org")
   (license license:gpl2+)))

(define-public dftoolbox
  (package
   (name "dftoolbox")
   (version current-commit)
   (source (local-file %source-dir
                       #:recursive? #t
                       #:select? (git-predicate %source-dir)))
   (arguments
    `(#:tests? #f
      #:phases
      (modify-phases %standard-phases
         (replace 'install
             (lambda* (#:key outputs #:allow-other-keys)
                (let* ((out (assoc-ref outputs "out")))
                  (invoke "make" "install"
                              (string-append "PREFIX=" out))))))))
   (build-system scons-build-system)
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
