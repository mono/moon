noinst_SCRIPTS = example-calculator
desklet = calculator.xap

noinst_DATA = default.xaml App.xaml calculator.xap

EXTRA_DIST = calculator.cs

example-calculator: Makefile
	echo "#!/bin/sh" > example-calculator
	echo "mopen -d $(desklet)" >> example-calculator
	chmod +x example-calculator

calculator.xap: calculator.cs default.xaml App.xaml
	PATH="../../../tools/xamlg/:../../../tools/respack/:$$PATH" mono ../../../tools/mxap/mxap.exe --tool-options="--uninstalled" --desktop --entry-point-type=Desklet.Calculator.App

CLEANFILES =			\
	example-calculator	\
	calculator.g.resources	\
	default.xaml.g.cs	\
	App.xaml.g.cs		\
	AppManifest.xaml	\
	calculator.xap		\
	calculator.dll		\
	calculator.dll.mdb
