if INSTALL_MONODEVELOP_SDK
SDK_ROOT=$(exec_prefix)/lib/moonlight
SDK2_DIR=$(DESTDIR)$(SDK_ROOT)/2.0
SDK2_REDIST_DIR=$(DESTDIR)$(SDK_ROOT)/2.0-redist

CLASS_DIR=$(top_builddir)/class/lib/2.1
SCRIPTS_DIR=$(top_builddir)/scripts
TOOLS_DIR=$(top_builddir)/tools

make-sdk-directories:
	@mkdir -p $(SDK2_DIR)
	@mkdir -p $(SDK2_REDIST_DIR)

install-data-local: make-sdk-directories install-buildversion install-sdk-assemblies install-sdk-tools install-redist-assemblies

install-buildversion:
	@echo $(SILVERLIGHT_2_0_TARGETRUNTIME) > buildversion
	$(INSTALL) -c -m 644 buildversion $(SDK2_DIR)/buildversion
	@rm buildversion

install-sdk-assemblies:
	$(INSTALL) -c $(CLASS_DIR)/mscorlib.dll $(SDK2_DIR)/mscorlib.dll
	$(INSTALL) -c $(CLASS_DIR)/mscorlib.dll.mdb $(SDK2_DIR)/mscorlib.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.dll $(SDK2_DIR)/System.dll
	$(INSTALL) -c $(CLASS_DIR)/System.dll.mdb $(SDK2_DIR)/System.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Core.dll $(SDK2_DIR)/System.Core.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Core.dll.mdb $(SDK2_DIR)/System.Core.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Net.dll $(SDK2_DIR)/System.Net.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Net.dll.mdb $(SDK2_DIR)/System.Net.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Runtime.Serialization.dll $(SDK2_DIR)/System.Runtime.Serialization.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Runtime.Serialization.dll.mdb $(SDK2_DIR)/System.Runtime.Serialization.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.ServiceModel.dll $(SDK2_DIR)/System.ServiceModel.dll
	$(INSTALL) -c $(CLASS_DIR)/System.ServiceModel.dll.mdb $(SDK2_DIR)/System.ServiceModel.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.ServiceModel.Web.dll $(SDK2_DIR)/System.ServiceModel.Web.dll
	$(INSTALL) -c $(CLASS_DIR)/System.ServiceModel.Web.dll.mdb $(SDK2_DIR)/System.ServiceModel.Web.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.dll $(SDK2_DIR)/System.Windows.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.dll.mdb $(SDK2_DIR)/System.Windows.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Browser.dll $(SDK2_DIR)/System.Windows.Browser.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Browser.dll.mdb $(SDK2_DIR)/System.Windows.Browser.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Xml.dll $(SDK2_DIR)/System.Xml.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Xml.dll.mdb $(SDK2_DIR)/System.Xml.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/Microsoft.VisualBasic.dll $(SDK2_DIR)/Microsoft.VisualBasic.dll
#	ideally we'd install the mdb file as well, but class/tuning/Makefile.am  r132251 claims that
#	the .mdb file crashes the linker.  let's just install the dll for now.
#	$(INSTALL) -c $(CLASS_DIR)/Microsoft.VisualBasic.dll.mdb $(SDK2_DIR)/Microsoft.VisualBasic.dll.mdb

install-sdk-tools:
	$(INSTALL) -c $(SCRIPTS_DIR)/smcs $(SDK2_DIR)/smcs
	$(INSTALL) -c $(CLASS_DIR)/smcs.exe $(SDK2_DIR)/smcs.exe
	$(INSTALL) -c $(CLASS_DIR)/smcs.exe.mdb $(SDK2_DIR)/smcs.exe.mdb
	$(INSTALL) -c $(CLASS_DIR)/Mono.CompilerServices.SymbolWriter.dll $(SDK2_DIR)/Mono.CompilerServices.SymbolWriter.dll
	$(INSTALL) -c $(CLASS_DIR)/Mono.CompilerServices.SymbolWriter.dll.mdb $(SDK2_DIR)/Mono.CompilerServices.SymbolWriter.dll.mdb
	$(INSTALL) -c $(TOOLS_DIR)/respack/respack.exe $(SDK2_DIR)/respack.exe
	$(INSTALL) -c $(TOOLS_DIR)/respack/respack.exe.mdb $(SDK2_DIR)/respack.exe.mdb

install-redist-assemblies:
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Controls.dll $(SDK2_REDIST_DIR)/System.Windows.Controls.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Controls.dll.mdb $(SDK2_REDIST_DIR)/System.Windows.Controls.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Controls.Data.dll $(SDK2_REDIST_DIR)/System.Windows.Controls.Data.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Windows.Controls.Data.dll.mdb $(SDK2_REDIST_DIR)/System.Windows.Controls.Data.dll.mdb
	$(INSTALL) -c $(CLASS_DIR)/System.Xml.Linq.dll $(SDK2_REDIST_DIR)/System.Xml.Linq.dll
	$(INSTALL) -c $(CLASS_DIR)/System.Xml.Linq.dll.mdb $(SDK2_REDIST_DIR)/System.Xml.Linq.dll.mdb

clean-local:
	-rm -rf buildversion
endif
