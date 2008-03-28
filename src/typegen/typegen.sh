#!/bin/sh

gmcs typegen.cs -r:System.dll,System.Xml.dll -debug:full

BASEDIR=`dirname $0`
cd "$BASEDIR/../"
mono --debug typegen/typegen.exe

cd "$BASEDIR"
rm -f typegen.exe
rm -f typegen.exe.mdb
