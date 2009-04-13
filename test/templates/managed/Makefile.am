
assembly = SampleTest

CLEANFILES = \
	$(assembly).xap	\
	$(assembly).dll	\
	$(assembly).dll.mdb	\
	AppManifest.xaml	\
	$(wildcard *.g.cs)	\
	$(assembly).g.resources


all: $(assembly).xap

sources = $(wildcard *.xaml) $(wildcard *.cs)

MONO_MOONLIGHT_UNITTESTING = $(top_builddir)/test/2.0/Mono.Moonlight.UnitTesting/Mono.Moonlight.UnitTesting.dll

Mono.Moonlight.UnitTesting.dll: $(MONO_MOONLIGHT_UNITTESTING)
	cp $(MONO_MOONLIGHT_UNITTESTING)* .

$(assembly).xap: $(sources) Mono.Moonlight.UnitTesting.dll
	-rm $(assembly).dll
	$(MONO) --debug $(top_builddir)/tools/mxap/mxap.exe --entry-point-type:Mono.Moonlight.$(assembly) --application-name:$(assembly) -r:$(MONO_MOONLIGHT_UNITTESTING) --builddirhack=$(top_builddir) --generate-html-


test: $(assembly).xap
	firefox $(assembly).html