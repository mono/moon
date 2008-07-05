
MCS = gmcs
RUNTIME = mono
MCSFLAGS = -debug+ -unsafe -r:System.Drawing.dll -r:Mono.Posix.dll -r:System.Data.dll -r:Mono.Data.SqliteClient.dll -r:$(srcdir)/NDesk.Options.dll -pkg:ndesk-dbus-1.0

CLEANFILES = test-runner.exe test-runner.exe.mdb logging-server.exe ndesk-options.flags

test_runner_sources = 		\
	Agviewer.cs		\
	DbusServices.cs		\
	IDbusService.cs		\
	Driver.cs		\
	ExternalProcess.cs	\
	Test.cs			\
	TestBeginEventHandler.cs	\
	TestCompleteEventHandler.cs	\
	TestCompleteReason.cs		\
	TestResult.cs		\
	TestRun.cs		\
	TestRunner.cs		\
	HtmlTest.cs		\
	XamlTest.cs		\
	LoggingServer.cs	\
	Screensaver.cs		\
	ImageCompare.cs		\
	EdgeCompare.cs		\
	IReport.cs		\
	VerboseLevel.cs		\
	ComparisonReport.cs	\
	ConsoleReport.cs	\
	HtmlReport.cs		\
	XmlReport.cs		\
	DbReport.cs

test_runner_resources = 				\
	/resource:$(srcdir)/html_template.html		\
	/resource:$(srcdir)/comparison_report.html

logging_server_sources =	\
	IDbusService.cs		\
	LoggingServer.cs	\
	TestResult.cs

exocortex_sources = 				\
	ExocortexDSP/src/AssemblyInfo.cs  	\
	ExocortexDSP/src/Complex.cs   		\
	ExocortexDSP/src/ComplexMath.cs   	\
	ExocortexDSP/src/FourierDirection.cs	\
	ExocortexDSP/src/ComplexArray.cs  	\
	ExocortexDSP/src/ComplexF.cs		\
	ExocortexDSP/src/ComplexStats.cs  	\
	ExocortexDSP/src/Fourier.cs


EXTRA_DIST = html_template.html comparison_report.html $(test_runner_sources) $(exocortex_sources) $(srcdir)/NDesk.Options.dll

all: test-runner.exe logging-server.exe

test-runner.exe: $(test_runner_sources) $(exocortex_sources)
	$(MCS) $(MCSFLAGS) $(test_runner_resources) /out:$@ $^ 

logging-server.exe: $(logging_server_sources)
	$(MCS) $(MCSFLAGS) -d:LOGGING_SERVER_STANDALONE /out:$@ $^

update-libraries:
	cp `pkg-config --variable=Libraries ndesk-options` .

setup-test-dir: all
	cp test-runner.exe $(TEST_DIR)/.
	cp html_template.html $(TEST_DIR)/.

setup-moon-test-dir: all
	cp test-runner.exe $(MOON_TEST_DIR)/.
	cp html_template.html $(MOON_TEST_DIR)/.

cleanup-test-dir:
	rm -rf $(TEST_DIR)/test-runner.exe
	rm -rf $(TEST_DIR)/html_template.html
