all: perf-suite-runner.exe tool

perf-suite-runner.exe: perf-suite-runner/DrtStore.cs perf-suite-runner/DrtItem.cs perf-suite-runner/Main.cs perf-suite-runner/Result.cs
	gmcs perf-suite-runner/Result.cs perf-suite-runner/DrtStore.cs perf-suite-runner/DrtItem.cs perf-suite-runner/Main.cs -out:perf-suite-runner.exe 

tool: perf-suite-tool
	gcc perf-suite-tool/perf-suite-tool.cpp -o tool `pkg-config --libs --cflags gtk+-2.0 mozilla-gtkmozembed mozilla-js moon`

clean:
	rm -f perf-suite-runner.exe
	rm -f tool

