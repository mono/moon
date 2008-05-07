SUBDIRS = . Mono.Moonlight System.Windows System.Windows.Browser

noinst_SCRIPTS = lib/2.1/smcs.exe

EXTRA_DIST = mono.pub winfx3.pub msfinal.pub mono.snk silverlight.pub

lib/2.1/smcs.exe:
	-mkdir -p lib/3.0
	-mkdir -p lib/2.1
	cp $(SMCS_ASM) lib/2.1

clean-local:
	-rm -rf lib
