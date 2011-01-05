#!/bin/bash -e

mkdir -p timecode2
cd timecode2
rm -f *

fps=24
seconds=2

s=0
f=0

BASENAME=timecode-${seconds}s-${fps}fps

while [[ $s < $seconds ]]; do
	
	if [[ $f -lt 10 ]]; then
		F=0$f;
	else
		F=$f;
	fi
	if [[ $s -lt 10 ]]; then
		S=0$s;
	else
		S=$s;
	fi
	convert -size 60x24 xc:white -pointsize 18 -draw "gravity Center text 0,0 '$S.$F'" $BASENAME-$S-$F.jpg
	let f=$f+1
	if [[ "x$fps" == "x$f" ]]; then
		f=0
		let s=$s+1
	fi
done

# make files follow a numerical sequence
x=1
for i in *.jpg; do 
	counter=$(printf %03d $x)
	ln $i $BASENAME-$counter.jpg
	x=$(($x+1))
done

# create mov file
ffmpeg -y -r $fps -f image2 -i $BASENAME-%03d.jpg $BASENAME.mov

# convert video to y4m for x264
ffmpeg -i $BASENAME.mov $BASENAME.y4m

# create mp4 files
x264 -o $BASENAME-h264.mp4 $BASENAME.y4m
x264 -o $BASENAME-h264-baseline.mp4 $BASENAME.y4m  --profile baseline
x264 -o $BASENAME-h264-main.mp4 $BASENAME.y4m  --profile main
x264 -o $BASENAME-h264-high.mp4 $BASENAME.y4m  --profile high

# create a mov file
ffmpeg -i timecode-2s-24fps-h264.mp4 -vcodec copy timecode-2s-24fps-h264.mov

# remove helper files
rm $BASENAME.mov $BASENAME.y4m
rm *.jpg

mv * ..
cd ..
rm -Rf timecode2