if INSTALL_MONODEVELOP_SDK
SDK_ROOT=$(exec_prefix)/lib/moonlight
SDK4_DIR=$(DESTDIR)$(SDK_ROOT)/4.0
SDK4_REDIST_DIR=$(DESTDIR)$(SDK_ROOT)/4.0-redist

CLASS_DIR=$(top_builddir)/class/lib/2.1
SCRIPTS_DIR=$(top_builddir)/scripts
TOOLS_DIR=$(top_builddir)/tools

make-sdk-directories:
	@mkdir -p $(SDK4_DIR)
	@mkdir -p $(SDK4_REDIST_DIR)

install-data-local: make-sdk-directories install-buildversion install-sdk-assemblies install-redist-assemblies install-compiler

install-buildversion:
	@echo $(SILVERLIGHT_4_0_TARGETRUNTIME) > buildversion
	$(INSTALL) -m 644 buildversion $(SDK4_DIR)/buildversion
	@rm buildversion

install-sdk-assemblies:
	$(INSTALL) $(CLASS_DIR)/mscorlib.dll $(SDK4_DIR)/mscorlib.dll
	-$(INSTALL) $(CLASS_DIR)/mscorlib.dll.mdb $(SDK4_DIR)/mscorlib.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.dll $(SDK4_DIR)/System.dll
	-$(INSTALL) $(CLASS_DIR)/System.dll.mdb $(SDK4_DIR)/System.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Core.dll $(SDK4_DIR)/System.Core.dll
	-$(INSTALL) $(CLASS_DIR)/System.Core.dll.mdb $(SDK4_DIR)/System.Core.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Net.dll $(SDK4_DIR)/System.Net.dll
	-$(INSTALL) $(CLASS_DIR)/System.Net.dll.mdb $(SDK4_DIR)/System.Net.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Runtime.Serialization.dll $(SDK4_DIR)/System.Runtime.Serialization.dll
	-$(INSTALL) $(CLASS_DIR)/System.Runtime.Serialization.dll.mdb $(SDK4_DIR)/System.Runtime.Serialization.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.ServiceModel.dll $(SDK4_DIR)/System.ServiceModel.dll
	-$(INSTALL) $(CLASS_DIR)/System.ServiceModel.dll.mdb $(SDK4_DIR)/System.ServiceModel.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.ServiceModel.Web.dll $(SDK4_DIR)/System.ServiceModel.Web.dll
	-$(INSTALL) $(CLASS_DIR)/System.ServiceModel.Web.dll.mdb $(SDK4_DIR)/System.ServiceModel.Web.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Windows.dll $(SDK4_DIR)/System.Windows.dll
	-$(INSTALL) $(CLASS_DIR)/System.Windows.dll.mdb $(SDK4_DIR)/System.Windows.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Windows.Browser.dll $(SDK4_DIR)/System.Windows.Browser.dll
	-$(INSTALL) $(CLASS_DIR)/System.Windows.Browser.dll.mdb $(SDK4_DIR)/System.Windows.Browser.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Xml.dll $(SDK4_DIR)/System.Xml.dll
	-$(INSTALL) $(CLASS_DIR)/System.Xml.dll.mdb $(SDK4_DIR)/System.Xml.dll.mdb
	$(INSTALL) $(CLASS_DIR)/Microsoft.VisualBasic.dll $(SDK4_DIR)/Microsoft.VisualBasic.dll
#	ideally we'd install the mdb file as well, but class/tuning/Makefile.am  r132251 claims that
#	the .mdb file crashes the linker.  let's just install the dll for now.
#	$(INSTALL) $(CLASS_DIR)/Microsoft.VisualBasic.dll.mdb $(SDK4_DIR)/Microsoft.VisualBasic.dll.mdb

install-redist-assemblies:
	$(INSTALL) $(CLASS_DIR)/System.Windows.Controls.dll $(SDK4_REDIST_DIR)/System.Windows.Controls.dll
	-$(INSTALL) $(CLASS_DIR)/System.Windows.Controls.dll.mdb $(SDK4_REDIST_DIR)/System.Windows.Controls.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Windows.Controls.Data.dll $(SDK4_REDIST_DIR)/System.Windows.Controls.Data.dll
	-$(INSTALL) $(CLASS_DIR)/System.Windows.Controls.Data.dll.mdb $(SDK4_REDIST_DIR)/System.Windows.Controls.Data.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Xml.Linq.dll $(SDK4_REDIST_DIR)/System.Xml.Linq.dll
	-$(INSTALL) $(CLASS_DIR)/System.Xml.Linq.dll.mdb $(SDK4_REDIST_DIR)/System.Xml.Linq.dll.mdb
	$(INSTALL) $(CLASS_DIR)/System.Xml.Serialization.dll $(SDK4_REDIST_DIR)/System.Xml.Serialization.dll
	-$(INSTALL) $(CLASS_DIR)/System.Xml.Serialization.dll.mdb $(SDK4_REDIST_DIR)/System.Xml.Serialization.dll.mdb

install-compiler:
	$(INSTALL) $(CLASS_DIR)/../mcs.exe $(SDK_ROOT)/mcs.exe

clean-local:
	-rm -rf buildversion
endif
