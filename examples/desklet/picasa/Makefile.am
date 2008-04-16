bin_SCRIPTS = example-picasa
deskletdir = $(libdir)/desklets/example-picasa

desklet_DATA = default.xaml picasa.dll Google.GData.Client.dll

EXTRA_DIST = default.xaml picasa.cs Google.GData.Client.dll

example-picasa: Makefile
	echo "#!/bin/sh" > example-picasa
	echo "mopen -d $(deskletdir)" >> example-picasa
	chmod +x example-picasa

picasa.dll: picasa.cs Google.GData.Client.dll $(top_builddir)/gtk/gtksilver.dll $(top_builddir)/class/lib/3.0/System.Windows.dll
	gmcs -debug -target:library -r:$(top_builddir)/gtk/gtksilver.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.dll picasa.cs -r:Google.GData.Client.dll

Google.GData.Client.dll:
	wget http://google-gdata.googlecode.com/files/gdata.net.1099.zip -O google.zip
	unzip -j -o google.zip root/clients/cs/lib/Release/Google.GData.Client.dll -d .
