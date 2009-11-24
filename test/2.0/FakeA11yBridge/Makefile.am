SMCS = smcs -noconfig -codepage:utf8 -keyfile:mono-uia.snk -target:library

if MOON_A11Y_INTERNAL_HACK
SMCS += -define:MOON_A11Y_INTERNAL_HACK
endif

CLEANFILES = \
	$(wildcard *.dll)	\
	$(wildcard *.mdb)

MoonAtkBridge.dll:
	$(SMCS) AttackVector.cs -out:$@

all: MoonAtkBridge.dll
