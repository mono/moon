#don't define MOONLIGHT_ONLY when compiling on MS.NET
SMCS = mcs -nostdlib -lib:../lib/2.1 -r:mscorlib.dll $(CSCFLAGS) -codepage:utf8 -keyfile:mono-uia.snk -target:library -define:MOONLIGHT_ONLY

EXTRA_DIST = \
	$(srcdir)/*.cs  \
	$(srcdir)/*.snk

CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)

MoonAtkBridge.dll:
	$(SMCS) AttackVector.cs -out:$@

all: MoonAtkBridge.dll
