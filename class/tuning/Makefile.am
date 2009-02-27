
LINKER = ../lib/tuner/monolinker.exe
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib /package 2.1

TUNER_DESCS =	\
	Descriptors/mscorlib.xml	\
	Descriptors/smcs.xml	\
	Descriptors/System.xml	\
	Descriptors/System.ServiceModel.xml

TUNER_ASSEMBLIES =	\
	mscorlib	\
	System	\
	System.Net	\
	System.Core	\
	System.Xml	\
	System.Xml.Linq	\
	System.Runtime.Serialization	\
	System.ServiceModel	\
	System.ServiceModel.Web	\
	Mono.CompilerServices.SymbolWriter

TUNER_MASTERS = $(TUNER_ASSEMBLIES:%=masterinfos/%.info)

EXTRA_DIST = $(TUNER_DESCS) $(TUNER_MASTERS)

WORKING_DIR = $(MCS_PATH)/class/lib/net_2_1_raw
OUTPUT_DIR = ../lib/2.1

DLLFILES = $(TUNER_ASSEMBLIES:%=%.dll)

FILES = smcs.exe $(DLLFILES)
WORKING_FILES = $(FILES:%=$(WORKING_DIR)/%)
OUTPUT_FILES = $(FILES:%=$(OUTPUT_DIR)/%)

TUNER_INSTALL_DIR = $(DESTDIR)$(prefix)/lib/mono/2.1

all-local: $(WORKING_FILES) $(OUTPUT_FILES)

install-data-local: $(TUNER_INSTALL_DIR) end-install

clean-local:
	rm -f $(OUTPUT_FILES) tune.stamp tune.stampt

uninstall-local:
	for i in $(DLLFILES); do $(GACUTIL) /u $$i; done
	rm -f 

TUNER_FLAGS =	\
	-d $(WORKING_DIR)	\
	-o $(OUTPUT_DIR)	\
	-l none		\
	-c link		\
	-a smcs		\
	-b true		\
	-g true		\
	-m secattrs SecurityAttributes	\
	-m display_internalized false

TUNER_STEPS = \
	Mono.Tuner.InjectSecurityAttributes,Mono.Tuner:OutputStep	\
	Mono.Tuner.AdjustVisibility,Mono.Tuner:OutputStep	\
	Mono.Tuner.PrintStatus,Mono.Tuner:OutputStep	\
	Mono.Tuner.RemoveSerialization,Mono.Tuner:OutputStep

$(OUTPUT_FILES): $(WORKING_FILES)
	MONO_PATH="../lib/tuner$(PLATFORM_PATH_SEPARATOR)$$MONO_PATH" mono --debug $(LINKER) $(TUNER_FLAGS) $(TUNER_DESCS:%=-x %) $(TUNER_STEPS:%=-s %) $(TUNER_MASTERS:%=-i %)
	for i in $(DLLFILES); do sn -q -R $(OUTPUT_DIR)/$$i ../mono.snk; done

$(TUNER_INSTALL_DIR):
	-mkdir -p $(TUNER_INSTALL_DIR)

end-install:	\
	inst-mscorlib.dll	\
	inst-smcs.exe	\
	gac-System	\
	gac-System.Net	\
	gac-System.Xml	\
	gac-System.Xml.Linq	\
	gac-System.Core	\
	gac-System.Runtime.Serialization	\
	gac-System.ServiceModel	\
	gac-System.ServiceModel.Web	\
	gac-Mono.CompilerServices.SymbolWriter

inst-%: $(OUTPUT_DIR)/%
	$(INSTALL) -c -m 755  $(OUTPUT_DIR)/$* $(TUNER_INSTALL_DIR)/$*
	test ! -f $(OUTPUT_DIR)/$*.mdb || $(INSTALL) -c -m 755  $(OUTPUT_DIR)/$*.mdb $(TUNER_INSTALL_DIR)/$*.mdb

gac-%: $(OUTPUT_DIR)/%.dll
	$(GACUTIL) /f /i $(OUTPUT_DIR)/$*.dll
