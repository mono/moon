#!/bin/sh
gmcs typegen.cs -r:System.dll,System.Xml.dll -debug:full
mono --debug typegen.exe
rm -f typegen.exe
rm -f typegen.exe.mdb