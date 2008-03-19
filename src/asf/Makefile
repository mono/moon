ASF_INCLUDES=-I/usr/local/include -I/usr/local/include/ffmpeg -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/alsa
ASF_COMPILER_ARGUMENTS=-MMD -Wall -g -O0 -DDEBUG -I../.. -I.prec $(ASF_INCLUDES) -c
ASF_DUMP_ARGUMENTS=-Ddump -DDEBUG
ASF_BIN=asf
ASF_LINK=-lglib-2.0 -lgcov  -Wl,--rpath -Wl,/mono/head/moon/src/.libs -Wl,--rpath -Wl,/mono/install/lib -Wl,--rpath -Wl,/mono/install/lib ../.libs/libmoon.so  /mono/install/lib/libmono.so

default: asf-generated.h compile
	g++ -o $(ASF_BIN) asf.o asf-generated.o asf-guids.o asf-structures.o asf-debug.o asf-test.o $(ASF_LINK)

compile:
	g++ $(ASF_COMPILER_ARGUMENTS) asf.cpp -o asf.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-generated.cpp -o asf-generated.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-guids.cpp -o asf-guids.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-structures.cpp -o asf-structures.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-debug.cpp -o asf-debug.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-test.cpp -o asf-test.o	

valgrind: default
	valgrind --tool=memcheck -v --leak-check=full --log-file=v.log --smc-check=all $(ASF_BIN) $(ASF_ARGS)

gdb: default
#	gdb --eval-command=run --args $(ASF_BIN) $(ASF_ARGS)
	gdb --args $(ASF_BIN) $(ASF_ARGS)

run: default
	$(ASF_BIN) $(ASF_ARGS)

asf-generated.h: generate asf.h asf.cs

generate: asf.h asf.cs
	gmcs asf.cs -out:asf-generate.exe
	mono --debug asf-generate.exe
	rm -f asf-generate.exe*

corrupt: default
	valgrind --tool=memcheck -v --leak-check=full --log-file=v.log --smc-check=all $(ASF_BIN) -in:a.wmv -test -corruption:2000

asf-dump: asf.cpp asf-generated.cpp asf-guids.cpp asf-structures.cpp asf-debug.cpp asf-test.cpp
	g++ $(ASF_COMPILER_ARGUMENTS) $(ASF_DUMP_ARGUMENTS) asf.cpp -o asf-d.o
	g++ $(ASF_COMPILER_ARGUMENTS) $(ASF_DUMP_ARGUMENTS) asf-generated.cpp -o asf-generated-d.o
	g++ $(ASF_COMPILER_ARGUMENTS) $(ASF_DUMP_ARGUMENTS) asf-guids.cpp -o asf-guids-d.o
	g++ $(ASF_COMPILER_ARGUMENTS) $(ASF_DUMP_ARGUMENTS) asf-structures.cpp -o asf-structures-d.o
	g++ $(ASF_COMPILER_ARGUMENTS) $(ASF_DUMP_ARGUMENTS) asf-debug.cpp -o asf-debug-d.o
	g++ $(ASF_COMPILER_ARGUMENTS)                       asf-test.cpp -o asf-test-d.o	
	g++ -o $(ASF_BIN)-dump asf-d.o asf-generated-d.o asf-guids-d.o asf-structures-d.o asf-debug-d.o asf-test-d.o $(ASF_LINK)
	
