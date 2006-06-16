#!/bin/sh

set -e

PREFIX=dftoolbox-`date -I`
OUTFILE=$PREFIX.zip

rm -vfr $PREFIX

mkdir $PREFIX
mkdir $PREFIX/src/
mkdir $PREFIX/doc/
for i in README TODO COPYING; do
    cp -v $i $PREFIX/${i}.txt
done
cp -v *.exe SConstruct df-directory.txt $PREFIX
cp -v dftoolbox-default.ini $PREFIX/dftoolbox.ini
cp src/*.?pp $PREFIX/src
cp doc/*.txt $PREFIX/doc

pushd $PREFIX
mkdir -p dreamfall-mods/yourmod/
pushd dreamfall-mods/yourmod/
unzip ../../../df-directories.zip
popd
popd

rm -vf $OUTFILE
cd $PREFIX
unix2dos src/*.?pp doc/*.txt dftoolbox.ini README.txt TODO.txt COPYING.txt SConstruct df-directory.txt
zip -r ../$OUTFILE  .

echo 
echo "Created $OUTFILE"

# EOF #
