#don't define MOONLIGHT_ONLY when compiling on MS.NET
CSCFLAGS = -codepage:utf8 -keyfile:mono-uia.snk -target:library -define:MOONLIGHT_ONLY

EXTRA_DIST = \
	$(srcdir)/*.cs  \
	$(srcdir)/*.snk

CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)

MoonAtkBridge.dll:
	$(SMCS) AttackVector.cs -out:$@

all: MoonAtkBridge.dll
