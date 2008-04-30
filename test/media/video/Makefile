ASF_TESTS = *.wmv

asf:
	cd ../../../src/asf/ && make

test-asf: asf
	@for t in $(ASF_TESTS); do \
		../../../src/asf/asf -test:$$t; \
	done
	
player:
	gmcs -debug -target:library -out:video-player.dll -r:../../../gtk/gtksilver.dll -r:../../../class/lib/3.0/System.Windows.dll ../../xaml/video-player.cs
