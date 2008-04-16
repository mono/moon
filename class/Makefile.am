SUBDIRS = . Mono.Moonlight System.Windows

noinst_SCRIPTS = lib/stamp lib/2.1/smcs.exe

lib/2.1/smcs.exe lib/stamp:
	-mkdir lib
	-mkdir lib/3.0
	-mkdir lib/2.1
	cp $(prefix)/lib/mono/2.1/smcs.exe lib/2.1
	> lib/stamp

clean-local:
	-rm -rf lib stamp