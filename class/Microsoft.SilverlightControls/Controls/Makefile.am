assembly = System.Windows.Controls
assemblysl = ../../lib/2.1/$(assembly).dll
#assemblydesktop = ../../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:System.Core -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../../lib/3.0
SMCS = MONO_PATH="../../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib

system_windows_sources =						\
Src/Resource.Designer.cs \
Src/Button/Button.cs \
Src/ButtonBase/ButtonBase.cs \
Src/ButtonBase/ClickMode.cs \
Src/CheckBox/CheckBox.cs \
Src/Common/DragCompletedEventArgs.cs \
Src/Common/DragDeltaEventArgs.cs \
Src/Common/DragEventArgs.cs \
Src/Common/DragStartedEventArgs.cs \
Src/Common/FontFamilyConverter.cs \
Src/Common/FontStretchConverter.cs \
Src/Common/FontStyleConverter.cs \
Src/Common/FontWeightConverter.cs \
Src/Common/KeyboardNavigation.cs \
Src/Common/NullableBoolConverter.cs \
Src/Common/RoutedPropertyChangedEventArgs.cs \
Src/Common/RoutedPropertyChangedEventHandler.cs \
Src/Common/SelectionChangedEventArgs.cs \
Src/Common/SelectionChangedEventHandler.cs \
Src/Common/TextDecorationCollectionConverter.cs \
Src/Common/TypeConverters.cs \
Src/Common/UriTypeConverter.cs \
Src/GlobalSuppressions.cs \
Src/Properties/AssemblyInfo.cs \
Src/RadioButton/RadioButton.cs \
Src/RepeatButton/RepeatButton.cs \
Src/Thumb/Thumb.cs \
Src/ToggleButton/ToggleButton.cs \
Src/ToolTip/ToolTip.cs \
Src/ToolTip/ToolTipService.cs \
Src/ScrollViewer/ScrollContentPresenter.cs \
Src/ScrollViewer/ScrollViewer.cs \
Src/ScrollBar/ScrollBar.cs \
Src/ScrollBar/ScrollBarVisibility.cs \
Src/ScrollBar/ScrollEventArgs.cs \
Src/ScrollBar/ScrollEventType.cs \
Src/RangeBase/RangeBase.cs \
Src/ListBox/DisplayMemberValueConverter.cs \
Src/ListBox/ListBox.cs \
Src/ListBox/ListBoxItem.cs \
Src/ListBox/SelectionMode.cs

#Src/HyperlinkButton/HyperlinkButton.cs

$(assemblysl): $(system_windows_sources) Makefile $(top_builddir)/class/lib/2.1/Mono.Moonlight.dll $(top_builddir)/class/lib/2.1/System.Windows.dll
	$(SMCS) -target:library -out:$@ $(system_windows_sources) -d:NET_2_1 -keyfile:$(srcdir)/../../silverlight.pub -delaysign+ -r:$(top_builddir)/class/lib/2.1/System.Windows.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.Browser.dll
	sn -q -R $@ $(srcdir)/../../mono.snk -r:$(top_builddir)/class/lib/2.1/System.Windows.dll -r:$(top_builddir)/class/lib/3.0/System.Windows.Browser.dll

#$(assemblydesktop): $(system_windows_sources) Makefile $(top_builddir)/class/lib/3.0/Mono.Moonlight.dll
#	$(GMCS) -target:library -out:$@ $(system_windows_sources) -keyfile:$(srcdir)/../../mono.snk

clean-local:
	-rm -rf $(assemblies) check-static check-dynamic
	-rm -rf *.exe *.mdb tester.cs

install-data-local:
#	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1

uninstall-local:
	$(GACUTIL) /u $(assembly)

check-code: check-static check-dynamic

check-static:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	> check-static

check-dynamic:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	mono --debug check-static.exe generate-test-code > tester.cs
	gmcs tester.cs -r:../lib/3.0/System.Windows.dll -debug:full -nowarn:219
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug tester.exe
