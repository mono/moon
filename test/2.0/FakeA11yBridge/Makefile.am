#don't define MOONLIGHT_ONLY when compiling on MS.NET
SMCS = smcs -noconfig -codepage:utf8 -keyfile:mono-uia.snk -target:library -define:MOONLIGHT_ONLY

CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)

MoonAtkBridge.dll:
	$(SMCS) AttackVector.cs -out:$@

all: MoonAtkBridge.dll
