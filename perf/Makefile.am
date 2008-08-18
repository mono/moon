bin_PROGRAMS = perf-tool

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

perf_suite_lib_sources =				\
	perf-suite-lib/Database.cs			\
	perf-suite-lib/DbEntry.cs			\
	perf-suite-lib/PassDbEntry.cs			\
	perf-suite-lib/ItemDbEntry.cs			\
	perf-suite-lib/ResultDbEntry.cs

perf_suite_runner_sources =				\
	perf-suite-runner/DrtStore.cs			\
	perf-suite-runner/DrtItem.cs			\
	perf-suite-runner/PerfSuiteRunner.cs		\
	perf-suite-runner/DrtResult.cs			\
	perf-suite-runner/Options.cs			\
	perf-suite-runner/AssemblyInfo.cs

perf_suite_generator_sources =				\
	perf-suite-generator/GraphGenerator.cs		\
	perf-suite-generator/HtmlGenerator.cs		\
	perf-suite-generator/PerfSuiteGenerator.cs	\
	perf-suite-generator/UtilFu.cs

perf-suite-lib.dll: $(perf_suite_lib_sources)
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_LIB_FLAGS) $(perf_suite_lib_sources) /target:library /out:$@

perf-suite-runner.exe: $(perf_suite_runner_sources) perf-suite-lib.dll
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_RUNNER_FLAGS) $(perf_suite_runner_sources) /out:$@ 

perf-suite-generator.exe: $(perf_suite_generator_sources) perf-suite-lib.dll
	$(MCS) $(MCS_COMMON_FLAGS) $(MCS_GENERATOR_FLAGS) $(perf_suite_generator_sources) /out:$@ 

all: perf-suite-lib.dll perf-suite-runner.exe perf-suite-generator.exe perf-tool
