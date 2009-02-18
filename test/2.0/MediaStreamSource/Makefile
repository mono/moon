FILES=$(wildcard *.cs site/*.html *.xaml)

test: mxap
	firefox site/index.html

mxap: $(FILES)
	mxap -res:../../media/audio/miguel.mp3,miguel.mp3
	mkdir -p site/ClientBin
	mv MediaStreamSource.xap site/ClientBin

	