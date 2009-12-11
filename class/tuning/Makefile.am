SUBDIRS=SecurityAttributes

LINKER = ../lib/tuner/monolinker.exe
#GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib /package 2.1

TUNER_DESCS =	\
	$(srcdir)/Descriptors/mscorlib.xml	\
	$(srcdir)/Descriptors/smcs.xml	\
	$(srcdir)/Descriptors/System.xml	\
	$(srcdir)/Descriptors/System.Net.xml	\
	$(srcdir)/Descriptors/System.Xml.xml	\
	$(srcdir)/Descriptors/System.ServiceModel.xml	\
	$(srcdir)/Descriptors/a11y.xml

SECURITY_ATTRIBUTES_INFO = \
	SecurityAttributes/mscorlib.secattr \
	SecurityAttributes/System.secattr \
	SecurityAttributes/System.Core.secattr \
	SecurityAttributes/System.Net.secattr \
	SecurityAttributes/System.Runtime.Serialization.secattr \
	SecurityAttributes/System.ServiceModel.secattr \
	SecurityAttributes/System.ServiceModel.Web.secattr \
	SecurityAttributes/System.Windows.Browser.secattr \
	SecurityAttributes/System.Windows.secattr \
	SecurityAttributes/System.Xml.secattr \
	SecurityAttributes/Microsoft.VisualBasic.secattr

MCS_ASSEMBLIES = \
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

MCS_DLLFILES = $(MCS_ASSEMBLIES:%=$(MCS_PATH)/class/lib/net_2_1_raw/%.dll)

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
MCS_OUTPUT_DIR = $(MCS_PATH)/class/lib/net_2_1

DLLFILES = $(TUNER_ASSEMBLIES:%=%.dll)

FILES = smcs.exe $(DLLFILES)
WORKING_FILES = $(FILES:%=$(WORKING_DIR)/%)
OUTPUT_FILES = $(FILES:%=$(OUTPUT_DIR)/%)
INPUT_FILES = $(MCS_PATH)/class/lib/net_2_1_raw/smcs.exe $(MCS_DLLFILES) $(MONO_BASIC_DLLFILES)

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
	-a smcs		\
	-b true		\
	-g true		\
	-m secattrs SecurityAttributes	\
	-m throw_on_visibility_check true	\
	-m display_internalized false

TUNER_STEPS = \
	Mono.Tuner.InjectSecurityAttributes,Mono.Tuner:OutputStep	\
	Mono.Tuner.AdjustVisibility,Mono.Tuner:OutputStep	\
	Mono.Tuner.PrintStatus,Mono.Tuner:OutputStep	\
	Mono.Tuner.RemoveSerialization,Mono.Tuner:OutputStep	\
	Mono.Tuner.CheckVisibility,Mono.Tuner

if HAVE_MONO_BASIC
$(MONO_BASIC_DLLFILES): 
	$(MAKE) -C $(MONO_BASIC_PATH)/tools/moonlight tune
endif

working.stamp: $(INPUT_FILES)
	-mkdir -p $(WORKING_DIR)
	for i in $(INPUT_FILES); do cp -p $$i $$i.mdb $(WORKING_DIR); done
	-rm $(WORKING_DIR)/Microsoft.VisualBasic.dll.mdb
	touch $(WORKING_FILES) working.stamp

output.stamp: working.stamp $(SECURITY_ATTRIBUTES_INFO)
	MONO_PATH="../lib/tuner:$$MONO_PATH" mono --debug $(LINKER) $(TUNER_FLAGS) $(TUNER_DESCS:%=-x %) $(TUNER_STEPS:%=-s %) $(TUNER_MASTERS:%=-i %)
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
