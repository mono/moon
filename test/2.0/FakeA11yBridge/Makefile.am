#don't define MOONLIGHT_ONLY when compiling on MS.NET
SMCS = $(MONO) --runtime=moonlight --security=temporary-smcs-hack $(top_builddir)/class/lib/2.1/smcs.exe -noconfig -codepage:utf8 -keyfile:mono-uia.snk -target:library -define:MOONLIGHT_ONLY

EXTRA_DIST = \
	$(srcdir)/*.cs  \
	$(srcdir)/*.snk

CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)

MoonAtkBridge.dll:
	MONO_PATH="$(top_builddir)/class/lib/2.1:$$MONO_PATH" $(SMCS) AttackVector.cs -out:$@

all: MoonAtkBridge.dll
