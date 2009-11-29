assembly = System.Windows.Controls.Data

if INCLUDE_BROWSER_MANAGED_CODE
assemblysl = ../lib/2.1/$(assembly).dll
endif

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/moonlight/$(assembly).dll
endif

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:System.Core -r:System.Windows -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/moonlight
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib
monoloc = `pkg-config --variable=libdir mono`/mono/moonlight

MSSDKCONTROLS=$(srcdir)/../Microsoft.SilverlightControls/SDKControls/Data/src

system_windows_controls_data_sources =						\
	$(srcdir)/../System.Windows/Assembly/MonoTODOAttribute.cs		\
	$(srcdir)/../Consts.cs							\
	$(MSSDKCONTROLS)/Extensions.cs						\
	$(MSSDKCONTROLS)/DoubleUtil.cs						\
	$(MSSDKCONTROLS)/Resource.Designer.cs					\
	$(MSSDKCONTROLS)/VisualStates.cs					\
	$(MSSDKCONTROLS)/Properties/AssemblyInfo.cs				\
	$(MSSDKCONTROLS)/CollectionView/CollectionViewError.cs			\
	$(MSSDKCONTROLS)/CollectionView/ListCollectionView.cs			\
	$(MSSDKCONTROLS)/CollectionView/SortFieldComparer.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridAutoGeneratingColumnEventArgs.cs	\
	$(MSSDKCONTROLS)/DataGrid/DataGridBeginningEditEventArgs.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridBoundColumn.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridCellCollection.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridCellCoordinates.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridCell.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridCellEventArgs.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridCellsPresenter.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridCheckBoxColumn.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumnCollection.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumn.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumnEventArgs.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumnHeadersPresenter.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumnHeader.xaml.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumnReorderingEventArgs.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridColumns.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridDataConnection.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridDetailsPresenter.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridDisplayData.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridEndingEditEventArgs.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridEnumerations.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridError.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridFillerColumn.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridFrozenGrid.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridLengthConverter.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridLength.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridPreparingCellForEditEventArgs.cs	\
	$(MSSDKCONTROLS)/DataGrid/DataGridRowDetailsEventArgs.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridRowEventArgs.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridRowHeader.xaml.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridRows.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridRowsPresenter.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridRow.xaml.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridSelectedItemsCollection.cs		\
	$(MSSDKCONTROLS)/DataGrid/DataGridTemplateColumn.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGridTextColumn.cs				\
	$(MSSDKCONTROLS)/DataGrid/DataGridValueConverter.cs			\
	$(MSSDKCONTROLS)/DataGrid/DataGrid.xaml.cs				\
	$(MSSDKCONTROLS)/DataGrid/IEditableCollection.cs			\
	$(MSSDKCONTROLS)/DataGrid/IndexToValueTable.cs				\
	$(MSSDKCONTROLS)/DataGrid/KeyboardHelper.cs				\
	$(MSSDKCONTROLS)/DataGrid/Range.cs					\
	$(MSSDKCONTROLS)/DataGrid/ResourceHelper.cs				\
	$(MSSDKCONTROLS)/DataGrid/TestHooks/DataGridCellInternalTestHook.cs	\
	$(MSSDKCONTROLS)/DataGrid/TestHooks/DataGridInternalTestHook.cs		\
	$(MSSDKCONTROLS)/DataGrid/TestHooks/DataGridRowInternalTestHook.cs	\
	$(MSSDKCONTROLS)/DataGrid/TypeHelper.cs					\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridAutomationPeer.cs		\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridCellAutomationPeer.cs	\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridColumnHeaderAutomationPeer.cs		\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridColumnHeadersPresenterAutomationPeer.cs	\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridDetailsPresenterAutomationPeer.cs		\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridItemAutomationPeer.cs			\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridRowAutomationPeer.cs			\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridRowHeaderAutomationPeer.cs			\
	$(MSSDKCONTROLS)/DataGrid/Automation/DataGridRowsPresenterAutomationPeer.cs		

EXTRA_DIST = $(system_windows_controls_data_sources) 			\
	ChangeLog

System.Windows.Controls.Resource.resources: $(MSSDKCONTROLS)/Resource.resx
	resgen2 $(MSSDKCONTROLS)/Resource.resx System.Windows.Controls.Resource.resources

$(assembly).g.resources: $(MSSDKCONTROLS)/themes/generic.xaml
	mono $(top_builddir)/tools/respack/respack.exe $@ $(MSSDKCONTROLS)/themes/generic.xaml,themes/generic.xaml

RESOURCES = 	\
	-res:$(MSSDKCONTROLS)/DataGrid/DataGridTextColumn.xaml,System.Windows.Controls.DataGrid.DataGridTextColumn.xaml	\
	-res:$(assembly).g.resources	\
	-res:System.Windows.Controls.Resource.resources

$(assemblysl): $(system_windows_controls_data_sources) Makefile $(top_builddir)/class/lib/2.1/System.Windows.dll $(assembly).g.resources System.Windows.Controls.Resource.resources
	$(SMCS) -target:library -out:$@ $(system_windows_controls_data_sources) -d:NET_2_1 -r:$(top_builddir)/class/lib/2.1/System.Windows.dll -r:System.Net -keyfile:$(srcdir)/../winfx3.pub -delaysign+ $(RESOURCES)
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_controls_data_sources) Makefile $(top_builddir)/class/lib/moonlight/System.Windows.dll
	$(GMCS) -target:library -out:$@ $(system_windows_controls_data_sources) -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) 
	-rm -rf *.dll *.mdb
	-rm -rf *.resources

if INCLUDE_BROWSER_MANAGED_CODE
install-sl:
else
install-sl:
endif

if INCLUDE_DESKTOP_MANAGED_CODE
install-desktop:
	$(GACUTIL) /i $(assemblydesktop) /f /package moonlight
else
install-desktop:
endif

install-data-local: install-sl install-desktop

uninstall-local:
	$(GACUTIL) /u $(assembly)

