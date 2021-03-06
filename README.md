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
you modifications. The insert tool will, once started, scan that
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

