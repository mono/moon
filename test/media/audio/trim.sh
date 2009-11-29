#!/bin/bash -ex

CHANNELS="L R LFE RS LS C"
for i in $CHANNELS; do
	sox BigBuckBunny-DVDMaster-$i.flac -t wavpcm BigBuckBunny-DVDMaster-$i-9-16.wav  trim 9 7
done