#!/bin/sh

set -e

PREFIX=dftoolbox-`date -I`-1
OUTFILE=$PREFIX.zip

rm -vfr $PREFIX

mkdir $PREFIX
mkdir $PREFIX/sound/
mkdir $PREFIX/icons/
mkdir $PREFIX/icons/large/
mkdir $PREFIX/src/
mkdir $PREFIX/doc/
for i in README TODO COPYING; do
    cp -v $i $PREFIX/${i}.txt
done
cp -v *.exe SConstruct df-directory.txt $PREFIX
cp -v dftoolbox-default.ini $PREFIX/dftoolbox.ini
cp src/*.?pp $PREFIX/src
cp doc/*.txt $PREFIX/doc
cp icons/*.png $PREFIX/icons
cp icons/*.gif $PREFIX/icons
cp icons/large/*.gif $PREFIX/icons/large/
cp icons/large/*.png $PREFIX/icons/large/
cp sound/*.wav $PREFIX/sound

rm -vf $OUTFILE
cd $PREFIX
unix2dos src/*.?pp doc/*.txt dftoolbox.ini README.txt TODO.txt COPYING.txt SConstruct df-directory.txt
zip -r ../$OUTFILE  .

echo 
echo "Created $OUTFILE"

# EOF #
