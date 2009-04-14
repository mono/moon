assembly = System.Windows.Controls

if INCLUDE_BROWSER_MANAGED_CODE
assemblysl = ../lib/2.1/$(assembly).dll
endif

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/moonlight/$(assembly).dll
endif

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:System.Windows -d:AGCLR -unsafe -r:System.Core.dll

GMCS = gmcs $(CSCFLAGS) -lib:../lib/moonlight
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib
monoloc = `pkg-config --variable=libdir mono`/mono/moonlight

MSSDKCONTROLS=$(srcdir)/../Microsoft.SilverlightControls/SDKControls/Extended/Src

system_windows_controls_sources =						\
	$(srcdir)/../System.Windows/Assembly/MonoTODOAttribute.cs		\
	$(srcdir)/../Consts.cs							\
	$(MSSDKCONTROLS)/Extensions.cs						\
	$(MSSDKCONTROLS)/Resource.Designer.cs					\
	$(MSSDKCONTROLS)/VisualStates.cs					\
	$(MSSDKCONTROLS)/Properties/AssemblyInfo.cs				\
	$(MSSDKCONTROLS)/Calendar/CalendarBlackoutDatesCollection.cs		\
	$(MSSDKCONTROLS)/Calendar/CalendarButton.cs				\
	$(MSSDKCONTROLS)/Calendar/Calendar.cs					\
	$(MSSDKCONTROLS)/Calendar/CalendarDateChangedEventArgs.cs		\
	$(MSSDKCONTROLS)/Calendar/CalendarDateRange.cs				\
	$(MSSDKCONTROLS)/Calendar/CalendarDayButton.cs				\
	$(MSSDKCONTROLS)/Calendar/CalendarItem.cs				\
	$(MSSDKCONTROLS)/Calendar/CalendarModeChangedEventArgs.cs		\
	$(MSSDKCONTROLS)/Calendar/CalendarSelectionMode.cs			\
	$(MSSDKCONTROLS)/Calendar/DateTimeHelper.cs				\
	$(MSSDKCONTROLS)/Calendar/DateTimeTypeConverter.cs			\
	$(MSSDKCONTROLS)/Calendar/KeyboardHelper.cs				\
	$(MSSDKCONTROLS)/Calendar/SelectedDatesCollection.cs			\
	$(MSSDKCONTROLS)/Calendar/TestHooks/CalendarInternalTestHook.cs	\
	$(MSSDKCONTROLS)/Calendar/TestHooks/MonthInternalTestHook.cs	\
	$(MSSDKCONTROLS)/DatePicker/DatePicker.cs				\
	$(MSSDKCONTROLS)/DatePicker/DatePickerDateValidationErrorEventArgs.cs	\
	$(MSSDKCONTROLS)/DatePicker/DatePickerFormat.cs				\
	$(MSSDKCONTROLS)/DatePicker/DatePickerTextBox.cs			\
	$(MSSDKCONTROLS)/DatePicker/TestHooks/DatePickerInternalTestHook.cs	\
	$(MSSDKCONTROLS)/GridSplitter/DragValidator.cs				\
	$(MSSDKCONTROLS)/GridSplitter/GridSplitter.cs				\
	$(MSSDKCONTROLS)/GridSplitter/PreviewControl.cs				\
	$(MSSDKCONTROLS)/GridSplitter/TestHooks/GridSplitterInternalTestHook.cs	\
	$(MSSDKCONTROLS)/TabControl/Dock.cs					\
	$(MSSDKCONTROLS)/TabControl/TabControl.cs				\
	$(MSSDKCONTROLS)/TabControl/TabItem.cs					\
	$(MSSDKCONTROLS)/TabControl/TabPanel.cs					\
	$(MSSDKCONTROLS)/TabControl/TestHooks/TabControlInternalTestHook.cs	\
	$(MSSDKCONTROLS)/TabControl/TestHooks/TabItemInternalTestHook.cs	\
	$(MSSDKCONTROLS)/Automation/CalendarAutomationPeer.cs			\
	$(MSSDKCONTROLS)/Automation/CalendarButtonAutomationPeer.cs		\
	$(MSSDKCONTROLS)/Automation/CalendarDayButtonAutomationPeer.cs		\
	$(MSSDKCONTROLS)/Automation/DatePickerAutomationPeer.cs			\
	$(MSSDKCONTROLS)/Automation/GridSplitterAutomationPeer.cs		\
	$(MSSDKCONTROLS)/Automation/TabControlAutomationPeer.cs			\
	$(MSSDKCONTROLS)/Automation/TabItemAutomationPeer.cs			


EXTRA_DIST = $(system_windows_controls_sources) 			\
	ChangeLog

XAML_RESOURCES = \
	$(MSSDKCONTROLS)/App.xaml,app.xaml	\
	$(MSSDKCONTROLS)/themes/generic.xaml,themes/generic.xaml

$(assembly).Resource.resources: $(MSSDKCONTROLS)/Resource.resx
	resgen2 $(MSSDKCONTROLS)/Resource.resx $(assembly).Resource.resources

$(assembly).g.resources: $(MSSDKCONTROLS)/App.xaml $(MSSDKCONTROLS)/themes/generic.xaml
	mono $(top_builddir)/tools/respack/respack.exe $@ $(XAML_RESOURCES)

$(assemblysl): $(system_windows_controls_sources) Makefile $(top_builddir)/class/lib/2.1/System.Windows.dll $(assembly).g.resources $(assembly).Resource.resources
	$(SMCS) -target:library -out:$@ $(system_windows_controls_sources) -d:NET_2_1 -r:$(top_builddir)/class/lib/2.1/System.Windows.dll -r:System.Net -keyfile:$(srcdir)/../winfx3.pub -delaysign+ -res:$(assembly).g.resources -res:$(assembly).Resource.resources
	sn -q -R $@ $(srcdir)/../mono.snk 

$(assemblydesktop): $(system_windows_controls_sources) Makefile $(top_builddir)/class/lib/moonlight/System.Windows.dll $(assembly).g.resources $(assembly).Resource.resources
	$(GMCS) -target:library -out:$@ $(system_windows_controls_sources) -res:$(assembly).g.resources -res:$(assembly).Resource.resources -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) 
	-rm -rf *.dll *.mdb

DISTCLEANFILES = \
	System.Windows.Controls.Resource.resources \
	System.Windows.Controls.g.resources

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

