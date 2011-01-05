tests.push ("embedded-script-commands.wmv");
results ["embedded-script-commands.wmv"] = [
"MediaOpened", 
"1 MarkerReached: ms = 933, type = caption, text = sub 1", 
"2 MarkerReached: ms = 1467, type = caption, text = sub 2", 
"3 MarkerReached: ms = 2133, type = text, text = some text here", 
"4 MarkerReached: ms = 2933, type = caption, text = sub 3", 
"5 MarkerReached: ms = 3533, type = caption, text = <center>sub 4</center>", 
"6 MarkerReached: ms = 4333, type = caption, text = <b>sub 5</b>", 
"MediaEnded"
]


tests.push ("timecode-mini-caption-all.wmv");
results ["timecode-mini-caption-all.wmv"] = [
"MediaOpened",
"1 MarkerReached: ms = 0, type = caption, text = 00:00",
"2 MarkerReached: ms = 50, type = caption, text = 00:01",
"3 MarkerReached: ms = 79, type = caption, text = 00:02",
"4 MarkerReached: ms = 117, type = caption, text = 00:03",
"5 MarkerReached: ms = 149, type = caption, text = 00:04",
"6 MarkerReached: ms = 187, type = caption, text = 00:05",
"7 MarkerReached: ms = 228, type = caption, text = 00:06",
"8 MarkerReached: ms = 254, type = caption, text = 00:07",
"9 MarkerReached: ms = 276, type = caption, text = 00:08",
"10 MarkerReached: ms = 324, type = caption, text = 00:09",
"11 MarkerReached: ms = 337, type = caption, text = 00:10",
"12 MarkerReached: ms = 384, type = caption, text = 00:11",
"13 MarkerReached: ms = 403, type = caption, text = 00:12",
"14 MarkerReached: ms = 445, type = caption, text = 00:13",
"15 MarkerReached: ms = 489, type = caption, text = 00:14",
"16 MarkerReached: ms = 521, type = caption, text = 00:15",
"17 MarkerReached: ms = 546, type = caption, text = 00:16",
"18 MarkerReached: ms = 588, type = caption, text = 00:17",
"19 MarkerReached: ms = 620, type = caption, text = 00:18",
"20 MarkerReached: ms = 658, type = caption, text = 00:19",
"21 MarkerReached: ms = 686, type = caption, text = 00:20",
"22 MarkerReached: ms = 721, type = caption, text = 00:21",
"23 MarkerReached: ms = 759, type = caption, text = 00:22",
"24 MarkerReached: ms = 798, type = caption, text = 00:23",
"25 MarkerReached: ms = 826, type = caption, text = 00:24",
"26 MarkerReached: ms = 855, type = caption, text = 00:25",
"27 MarkerReached: ms = 883, type = caption, text = 00:26",
"28 MarkerReached: ms = 918, type = caption, text = 00:27",
"29 MarkerReached: ms = 960, type = caption, text = 00:28",
"30 MarkerReached: ms = 995, type = caption, text = 00:29",
"31 MarkerReached: ms = 1020, type = caption, text = 01:00",
"32 MarkerReached: ms = 1052, type = caption, text = 01:01",
"MediaEnded"
];

tests.push ("timecode-short-caption-1.wmv");
results ["timecode-short-caption-1.wmv"] = [
"MediaOpened",
"1 MarkerReached: ms = 1004, type = caption, text = one sec",
"MediaEnded"
];

tests.push ("timecode-short-mpeg4.wmv");
results ["timecode-short-mpeg4.wmv"] = ["MediaFailed"];

tests.push ("timecode-short-vc1-adv.wmv");
results ["timecode-short-vc1-adv.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-vc1-main.wmv");
results ["timecode-short-vc1-main.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-vc1-simple.wmv");
results ["timecode-short-vc1-simple.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short.wmv");
results ["timecode-short.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv7.wmv");
results ["timecode-short-wmv7.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv8.wmv");
results ["timecode-short-wmv8.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv9src.wmv");
results ["timecode-short-wmv9src.wmv"] = ["MediaFailed"];

tests.push ("timecode-short-wmv9.wmv");
results ["timecode-short-wmv9.wmv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode.wmv");
results ["timecode.wmv"] = ["MediaFailed"];

/*
  ffmpeg -y -vframes 61 -i timecode.wmv -s 208x160 timecode-short.y4m
  x264 -o timecode-short-h264.mp4 timecode-short.y4m
  x264 -o timecode-short-h264-baseline.mp4 timecode-short.y4m  --profile baseline
  x264 -o timecode-short-h264-main.mp4 timecode-short.y4m  --profile main
  x264 -o timecode-short-h264-high.mp4 timecode-short.y4m  --profile high
  ffmpeg -y -i timecode-short.wmv timecode-short-mp43.mp4 -vcodec mp43
  rm -f timecode-short.y4m
*/
tests.push ("timecode-short-h264.mp4");
results ["timecode-short-h264.mp4"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-h264-baseline.mp4");
results ["timecode-short-h264-baseline.mp4"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-h264-main.mp4");
results ["timecode-short-h264-main.mp4"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-h264-high.mp4");
results ["timecode-short-h264-high.mp4"] = ["MediaOpened", "MediaEnded"];

// MSDN says: "Supports H.264 and MP43 codecs."  http://msdn.microsoft.com/en-us/library/cc189080(VS.95).aspx
// SL doesn't agree.
tests.push ("timecode-short-mp43.mp4");
results ["timecode-short-mp43.mp4"] = ["MediaFailed"];

