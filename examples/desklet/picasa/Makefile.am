bin_SCRIPTS = example-picasa
deskletdir = $(libdir)/desklets/example-picasa

desklet_DATA = default.xaml picasa.dll Google.GData.Client.dll

EXTRA_DIST = default.xaml picasa.cs Google.GData.Client.dll

example-picasa: Makefile
	echo "#!/bin/sh" > example-picasa
	echo "mopen -d $(deskletdir)" >> example-picasa
	chmod +x example-picasa

picasa.dll: picasa.cs Google.GData.Client.dll
	gmcs -debug -target:library -pkg:silverdesktop -r:../../../gtk/gtksilver.dll picasa.cs -r:Google.GData.Client.dll

Google.GData.Client.dll:
	wget http://google-gdata.googlecode.com/files/gdata.net.1099.zip -O google.zip
	unzip -j -o google.zip root/clients/cs/lib/Release/Google.GData.Client.dll -d .
