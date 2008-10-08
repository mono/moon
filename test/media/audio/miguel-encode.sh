#!/bin/bash -e

lame -S --resample 16     -b 56    miguel.mp3 miguel-16kHz-56kbps.mp3
lame -S --resample 44.1   -b 56    miguel.mp3 miguel-44.1kHz-56kbps.mp3
lame -S --resample 11.025 -b 16    miguel.mp3 miguel-11.025kHz-16kbps.mp3
lame -S --resample 11.025 -b 16 -p miguel.mp3 miguel-11.025kHz-16kbps-p.mp3
