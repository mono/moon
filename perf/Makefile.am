if INCLUDE_PERFORMANCE
PERFORMANCE = true
else
PERFORMANCE = echo ERROR: Performance suite is disabled by configuration; false
endif

noinst_PROGRAMS = perf-tool

MOZ_PATH = `pwd`/$(top_builddir)/plugin/.libs
LD_PATH = $(top_builddir)/plugin/.libs:$(top_builddir)/src/.libs:`pkg-config --variable=sdkdir mozilla-gtkmozembed`/lib
MOON_PLUGIN_DIR = `pwd`/$(top_builddir)/plugin/.libs

INCLUDES = $(PERF_TOOL_CFLAGS) $(MOON_PROG_CFLAGS) -Wall -DG_LOG_DOMAIN=\"perf-tool\"

perf_tool_SOURCES =					\
	perf-suite-tool/perf-suite-tool.cpp

perf_tool_LDADD = $(PERF_TOOL_LIBS) $(MOON_PROG_LIBS)

MCS = gmcs
RUNTIME = mono
MCS_COMMON_FLAGS = -r:Mono.GetOptions

MCS_LIB_FLAGS = -r:Mono.Data.SqliteClient -r:System.Data
MCS_RUNNER_FLAGS = -r:perf-suite-lib.dll
MCS_GENERATOR_FLAGS = -r:perf-suite-lib.dll -r:Mono.Cairo

check-performance:
	@$(PERFORMANCE)	

perf_suite_lib_sources = \
	$(srcdir)/perf-suite-lib/Database.cs			\
	$(srcdir)/perf-suite-lib/DbEntry.cs			\
	$(srcdir)/perf-suite-lib/PassDbEntry.cs			\
	$(srcdir)/perf-suite-lib/ItemDbEntry.cs			\
	$(srcdir)/perf-suite-lib/ResultDbEntry.cs

perf_suite_runner_sources = \
	$(srcdir)/perf-suite-runner/DrtStore.cs			\
	$(srcdir)/perf-suite-runner/DrtItem.cs			\
	$(srcdir)/perf-suite-runner/PerfSuiteRunner.cs		\
	$(srcdir)/perf-suite-runner/DrtResult.cs		\
	$(srcdir)/perf-suite-runner/Options.cs			\
	$(srcdir)/perf-suite-runner/AssemblyInfo.cs

perf_suite_generator_sources = \
	$(srcdir)/perf-suite-generator/GraphGenerator.cs	\
	$(srcdir)/perf-suite-generator/HtmlGenerator.cs		\
	$(srcdir)/perf-suite-generator/PerfSuiteGenerator.cs	\
	$(srcdir)/perf-suite-generator/UtilFu.cs		\
	$(srcdir)/perf-suite-generator/AssemblyInfo.cs		\
	$(srcdir)/perf-suite-generator/Options.cs

perf-suite-lib.dll: $(perf_suite_lib_sources) check-performance
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_LIB_FLAGS) $(perf_suite_lib_sources) /target:library /out:$@

perf-suite-runner.exe: $(perf_suite_runner_sources) perf-suite-lib.dll check-performance
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_RUNNER_FLAGS) $(perf_suite_runner_sources) /out:$@ 

perf-suite-generator.exe: $(perf_suite_generator_sources) perf-suite-lib.dll check-performance
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_GENERATOR_FLAGS) $(perf_suite_generator_sources) /out:$@ 

all: perf-suite-lib.dll perf-suite-runner.exe perf-suite-generator.exe perf-tool

run-perf: all check-performance
	GNOME_DISABLE_CRASH_DIALOG=1 MOON_PLUGIN_DIR=$(MOON_PLUGIN_DIR) MOZ_PLUGIN_PATH=$(MOZ_PATH) LD_LIBRARY_PATH=$(LD_PATH):$(LD_LIBRARY_PATH) $(RUNTIME) perf-suite-runner.exe
	$(RUNTIME) perf-suite-generator.exe

EXTRA_DIST = $(perf_suite_lib_sources) $(perf_suite_runner_sources) $(perf_suite_generator_sources) perf-report/helpers.js perf-report/jquery.js  perf-report/logo.png  perf-report/report.css perf-suite-set

CLEANFILES = perf-suite-lib.dll perf-suite-runner.exe perf-suite-generator.exe

.PHONY: check-performance
