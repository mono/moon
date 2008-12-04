noinst_DATA = unsign.exe

CLEANFILES= unsign.exe \
	    unsign.exe.mdb

EXTRA_DIST=unsign.cs

unsign.exe: unsign.cs
	gmcs unsign.cs -target:exe -out:unsign.exe -debug:full -r:`pkg-config --variable=Libraries cecil`
