noinst_DATA = unsign.exe

CECIL=$(MCS_PATH)/class/lib/basic/Mono.Cecil.dll

CLEANFILES = unsign.exe \
	     unsign.exe.mdb

EXTRA_DIST = $(unsign_exe_CS)

unsign_exe_CS = $(srcdir)/unsign.cs

unsign.exe: $(unsign_exe_CS) 
	gmcs $(unsign_exe_CS) -target:exe -out:unsign.exe -debug:full -r:$(CECIL)
