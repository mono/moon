ASF_COMPILER_ARGUMENTS=-MMD -Wall -g -O0 -DDEBUG -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I../.. -I.prec -c
ASF_BIN=asf

default: asf-generated.h compile
	g++ -o $(ASF_BIN) asf.o asf-generated.o asf-guids.o asf-structures.o asf-debug.o asf-test.o -lglib-2.0

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

