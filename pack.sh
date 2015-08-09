#!/bin/sh

set -e

make

rm -fr icfpc2015-n6b
mkdir icfpc2015-n6b
git archive master | tar -C icfpc2015-n6b -xvf -

tar -cvzf icfpc2015-n6b.tar.gz icfpc2015-n6b
