SUBDIRS=

LINKER = monolinker
#GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib /package 2.1

TUNER_DESCS =	\
	$(srcdir)/Descriptors/mscorlib.xml	\
	$(srcdir)/Descriptors/System.xml	\
	$(srcdir)/Descriptors/System.Net.xml	\
	$(srcdir)/Descriptors/System.Xml.xml	\
	$(srcdir)/Descriptors/System.ServiceModel.xml	\
	$(srcdir)/Descriptors/System.ServiceModel.Web.xml	\
	$(srcdir)/Descriptors/System.Core.xml	\
	$(srcdir)/Descriptors/a11y.xml

MCS_ASSEMBLIES = \
	mscorlib	\
	System	\
	System.Net	\
	System.Core	\
	System.Xml	\
	System.Xml.Linq	\
	System.Xml.Serialization	\
	System.Runtime.Serialization	\
	System.ServiceModel	\
	System.ServiceModel.Web

MCS_DLLFILES = $(MCS_ASSEMBLIES:%=$(MCS_PATH)/class/lib/moonlight_raw/%.dll)

MCS_COMPILER = $(MCS_PATH)/mcs/mcs.exe

if HAVE_MONO_BASIC
MONO_BASIC_TUNER_ASSEMBLIES = Microsoft.VisualBasic
MONO_BASIC_DLLFILES = $(MONO_BASIC_PATH)/class/lib/2.1/Microsoft.VisualBasic.dll
endif

TUNER_ASSEMBLIES = $(MCS_ASSEMBLIES) $(MONO_BASIC_TUNER_ASSEMBLIES)

TUNER_MASTERS = $(TUNER_ASSEMBLIES:%=$(srcdir)/masterinfos/%.info)

EXTRA_DIST = $(TUNER_DESCS) $(TUNER_MASTERS)

# We copy all the files to be tuned into our working directory
# this way it's easier to tune assemblies which comes from 
# different sources (mcs, mono-basic)
WORKING_DIR = ../lib/2.1_raw
OUTPUT_DIR = ../lib/2.1
MCS_OUTPUT_DIR = $(MCS_PATH)/class/lib/moonlight

DLLFILES = $(TUNER_ASSEMBLIES:%=%.dll)

FILES = $(DLLFILES)
WORKING_FILES = $(FILES:%=$(WORKING_DIR)/%)
OUTPUT_FILES = $(FILES:%=$(OUTPUT_DIR)/%)
INPUT_FILES = $(MCS_DLLFILES) $(MONO_BASIC_DLLFILES)

TUNER_INSTALL_DIR = $(DESTDIR)$(prefix)/lib/mono/2.1

all-local: output.stamp

clean-local:
	rm -f $(OUTPUT_FILES) tune.stamp tune.stampt working.stamp output.stamp

install-data-local: end-install

uninstall-local:
	for i in $(DLLFILES); do $(GACUTIL) /u $$i; done
	rm -f 

TUNER_FLAGS =	\
	-d $(WORKING_DIR)	\
	-o $(OUTPUT_DIR)	\
	-l none		\
	-c link		\
	-b true		\
	-g true		\
	-m throw_on_visibility_error true	\
	-m display_internalized false

TUNER_STEPS = \
	Mono.Tuner.AdjustVisibility,Mono.Tuner:OutputStep	\
	Mono.Tuner.PrintStatus,Mono.Tuner:OutputStep	\
	Mono.Tuner.RemoveSerialization,Mono.Tuner:OutputStep	\
	Mono.Tuner.CheckVisibility,Mono.Tuner

$(top_srcdir)/class/lib/tuner/Mono.Tuner.dll: $(wildcard $(MCS_PATH)/tools/tuner/Mono.Tuner/*.cs)
	mkdir -p $(top_srcdir)/class/lib/tuner/
	cd $(MCS_PATH)/tools/tuner && mcs -r:System.dll -r:System.Core.dll -r:System.Xml.dll -r:monolinker.exe -r:`pkg-config --variable=Libraries cecil` -target:library -out:$(PWD)/$@ @$(MCS_PATH)/tools/tuner/Mono.Tuner.dll.sources -debug:full

if HAVE_MONO_BASIC
$(MONO_BASIC_DLLFILES): 
	$(MAKE) -C $(MONO_BASIC_PATH)/tools/moonlight tune
endif

working.stamp: $(INPUT_FILES) $(MCS_COMPILER)
	-mkdir -p $(WORKING_DIR)
	for i in $(INPUT_FILES); do cp -p $$i $$i.mdb $(WORKING_DIR); done
	-rm $(WORKING_DIR)/Microsoft.VisualBasic.dll.mdb
	cp $(MCS_COMPILER) ../lib
	touch $(WORKING_FILES) working.stamp

output.stamp: working.stamp $(top_srcdir)/class/lib/tuner/Mono.Tuner.dll $(MCS_COMPILER)
	MONO_PATH="../lib/tuner:$$MONO_PATH" $(LINKER) $(TUNER_FLAGS) $(TUNER_DESCS:%=-x %) $(TUNER_STEPS:%=-s %) $(TUNER_MASTERS:%=-i %)
	for i in $(DLLFILES); do sn -q -R $(OUTPUT_DIR)/$$i $(srcdir)/../mono.snk; done
	-mkdir -p $(MCS_OUTPUT_DIR)
	for i in $(OUTPUT_FILES); do cp -p $$i $(MCS_OUTPUT_DIR); done
	for i in $(OUTPUT_FILES); do cp -p $$i.mdb $(MCS_OUTPUT_DIR) || true; done
	touch output.stamp

END_INSTALL_TARGETS = \
	inst-mscorlib.dll	\
	inst-System.dll	\
	inst-System.Net.dll	\
	inst-System.Xml.dll	\
	inst-System.Core.dll	\
	inst-System.Runtime.Serialization.dll	\
	inst-System.ServiceModel.dll	\
	inst-System.ServiceModel.Web.dll

if HAVE_MONO_BASIC
END_INSTALL_TARGETS += inst-Microsoft.VisualBasic.dll
endif

end-install: $(END_INSTALL_TARGETS)

inst-%: $(OUTPUT_DIR)/%
	-mkdir -p $(DESTDIR)$(pkglibdir)/plugin/
	$(INSTALL) -c -m 755  $(OUTPUT_DIR)/$* $(DESTDIR)$(pkglibdir)/plugin/$*
	test ! -f $(OUTPUT_DIR)/$*.mdb || $(INSTALL) -c -m 755  $(OUTPUT_DIR)/$*.mdb $(DESTDIR)$(pkglibdir)/plugin/$*.mdb

retune:
	rm -f output.stamp && $(MAKE) output.stamp
