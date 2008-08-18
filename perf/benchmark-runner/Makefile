benchmark-runner: benchmark-runner.cpp
	gcc benchmark-runner.cpp -o benchmark-runner `pkg-config --libs --cflags gtk+-2.0 mozilla-gtkmozembed mozilla-js moon`

clean:
	rm -f benchmark-runner

