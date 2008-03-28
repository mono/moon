#!/bin/sh
gmcs -debug fullscreen.cs -target:exe
mono --debug fullscreen.exe
rm -f fullscreen.exe*
