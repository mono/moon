ASF_COMPILER_ARGUMENTS=-MMD -Wall -g -O0 -DDEBUG -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I../.. -I.prec -c
ASF_BIN=asf.bin

default: asf-generated.h
	g++ $(ASF_COMPILER_ARGUMENTS) asf.cpp -o asf.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-generated.cpp -o asf-generated.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-guids.cpp -o asf-guids.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-structures.cpp -o asf-structures.o
	g++ $(ASF_COMPILER_ARGUMENTS) debug.cpp -o debug.o
	g++ -o $(ASF_BIN) asf.o asf-generated.o asf-guids.o asf-structures.o debug.o -lglib-2.0

debug: default
	gdb --eval-command=run --args $(ASF_BIN)

valgrind: default
	valgrind --tool=memcheck -v --leak-check=full --log-file=v.log --smc-check=all $(ASF_BIN)

gdb: default
	gdb --eval-command=run --args $(ASF_BIN)

run: default
	$(ASF_BIN)

asf-generated.h: generate asf.h asf.cs
	

generate:
	gmcs asf.cs -out:asf-generate.exe
	mono --debug asf-generate.exe
	rm -f asf-generate.exe*
