DESKLET = picasa
SOURCES = picasa.cs
LIBS = bin/Google.GData.Client.dll

include ../rules.make

download-google:
	rm -rf /tmp/google
	mkdir -p /tmp/google
	mkdir -p bin
	wget http://google-gdata.googlecode.com/files/gdata.net.1099.zip -O /tmp/google/google.zip
	unzip -j -o /tmp/google/google.zip root/clients/cs/lib/Release/Google.GData.Client.dll -d bin
	rm -rf /tmp/google

$(ASSEMBLY): $(DSOURCES) $(LIBS)
	mkdir -p bin
	gmcs -debug -target:library -r:$(LIBS) -pkg:silver -out:$(ASSEMBLY) $(DSOURCES)

