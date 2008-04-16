#!/bin/sh -e

pushd .

SRC=`dirname $0`/..
cd $SRC

gmcs typegen/typegen.cs -r:System.dll,System.Xml.dll -debug:full -out:typegen/typegen.exe
mono --debug typegen/typegen.exe
rm -f typegen/typegen.exe*

popd