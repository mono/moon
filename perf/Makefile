all: perf-suite-runner.exe perf-suite-lib.dll tool perf-suite-generator.exe

perf-suite-runner.exe: perf-suite-runner/DrtStore.cs perf-suite-runner/DrtItem.cs perf-suite-runner/Main.cs perf-suite-runner/DrtResult.cs perf-suite-lib.dll
	gmcs perf-suite-runner/DrtResult.cs perf-suite-runner/DrtStore.cs perf-suite-runner/DrtItem.cs perf-suite-runner/Main.cs -out:perf-suite-runner.exe -r:perf-suite-lib.dll

perf-suite-generator.exe: perf-suite-generator/GraphGenerator.cs perf-suite-generator/HtmlGenerator.cs perf-suite-generator/Main.cs perf-suite-lib.dll perf-suite-generator/UtilFu.cs
	gmcs perf-suite-generator/GraphGenerator.cs perf-suite-generator/HtmlGenerator.cs perf-suite-generator/Main.cs perf-suite-generator/UtilFu.cs -out:perf-suite-generator.exe -r:perf-suite-lib.dll -r:Mono.Cairo

perf-suite-lib.dll: perf-suite-lib/Database.cs perf-suite-lib/DbEntry.cs perf-suite-lib/PassDbEntry.cs perf-suite-lib/ItemDbEntry.cs perf-suite-lib/ResultDbEntry.cs
	gmcs perf-suite-lib/ResultDbEntry.cs perf-suite-lib/Database.cs perf-suite-lib/DbEntry.cs perf-suite-lib/PassDbEntry.cs perf-suite-lib/ItemDbEntry.cs -out:perf-suite-lib.dll -r:Mono.Data.SqliteClient -r:System.Data -target:library

tool: perf-suite-tool
	gcc perf-suite-tool/perf-suite-tool.cpp -o tool `pkg-config --libs --cflags gtk+-2.0 mozilla-gtkmozembed mozilla-js moon`

clean:
	rm -f perf-suite-runner.exe
	rm -f tool

