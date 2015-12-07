#!/bin/sh

echo "Creating testdir..\c"
rm -rf testdir
echo ".\c"
mkdir testdir
echo ".Done."


echo "Checking for knowndata binary...\c"
if [[ ! -x knowndata ]]
then
    echo ".\c"
    gcc -Wall -Werror -o knowndata knowndata.c
fi
echo "Done."


if [[ ! -x blog ]]
then
    gmake blog
fi

echo "Moving files in..\c"
cp blog testdir
echo ".\c"
cp knowndata testdir
echo "Done."
