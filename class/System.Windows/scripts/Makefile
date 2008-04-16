check-static:
	cd .. && make install
	gmcs checks.cs -pkg:silver -debug:full
	mono --debug checks.exe

check-dynamic:
	cd .. && make install
	gmcs checks.cs -pkg:silver -debug:full
	mono --debug checks.exe generate-test-code > tester.cs
	gmcs tester.cs -pkg:silver -debug:full -nowarn:219
	mono --debug tester.exe

check: check-static check-dynamic

clean:
	rm -f *.exe
	rm -f *.mdb
	rm tester.cs
