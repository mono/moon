thisdir = class/agclr
include ../../build/rules.make

LIBRARY = System.Windows.dll

LIB_MCS_FLAGS = -r:System -r:Mono.Moonlight -d:AGCLR -unsafe

include ../../build/library.make

check:
	cd scripts && make check
