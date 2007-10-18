ASF_COMPILER_ARGUMENTS=-MMD -Wall -g -O0 -DDEBUG -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I.prec -c

default: asf-generated.h
	g++ $(ASF_COMPILER_ARGUMENTS) asf.cpp -o asf.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-generated.cpp -o asf-generated.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-guids.cpp -o asf-guids.o
	g++ $(ASF_COMPILER_ARGUMENTS) asf-structures.cpp -o asf-structures.o
	g++ -o asf *.o -lglib-2.0

debug: default
	gdb --eval-command=run --args asf

valgrind: default
	valgrind --tool=memcheck -v --leak-check=full --log-file=v.log --smc-check=all asf

run: default
	asf

asf-generated.h: generate asf.h asf.cs
	

generate:
	gmcs *.cs -out:asf-generate.exe
	mono --debug asf-generate.exe
	rm -f asf-generate.exe*
