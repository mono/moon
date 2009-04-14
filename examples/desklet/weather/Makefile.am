bin_SCRIPTS = example-weather
deskletdir = $(libdir)/desklets/example-weather

desklet_DATA = default.xaml weather.dll

sources = \
	$(srcdir)/Locations.cs 		\
	$(srcdir)/LocationsGenerated.cs \
	$(srcdir)/Metar.cs 		\
	$(srcdir)/default.xaml.cs 	\
	$(srcdir)/config.cs

EXTRA_DIST = $(sources) default.xaml StationCompiler.cs data/Locations.xml.bz2

example-weather: Makefile
	echo "#!/bin/sh" > example-weather
	echo "mopen -d $(deskletdir)" >> example-weather
	chmod +x example-weather

weather.dll: $(sources) $(top_builddir)/gtk/Moonlight.Gtk.dll $(top_builddir)/class/lib/moonlight/System.Windows.dll
	gmcs -debug -out:$@ -target:library -out:weather.dll -r:$(top_builddir)/gtk/Moonlight.Gtk.dll -r:$(top_builddir)/class/lib/moonlight/System.Windows.dll -pkg:gtk-sharp-2.0 -pkg:gconf-sharp-2.0 $(sources)

LocationsGenerated.cs: $(srcdir)/StationCompiler.cs $(srcdir)/data/Locations.xml.bz2
	gmcs $(srcdir)/StationCompiler.cs 
	bunzip2 -c data/Locations.xml.bz2 > Locations.xml
	mono StationCompiler.exe Locations.xml LocationsGenerated.cs

CLEANFILES =			\
	example-weather		\
	weather.dll		\
	weather.dll.mdb
