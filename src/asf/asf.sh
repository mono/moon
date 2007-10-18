gmcs asf.cs -out:asf.exe -debug:full
mono --debug asf.exe
rm -f asf.exe*
