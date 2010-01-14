
tests.push ("embedded-script-commands");
results ["embedded-script-commands"] = [
"MediaOpened", 
"1 MarkerReached: ms = 933, type = caption, text = sub 1", 
"2 MarkerReached: ms = 1467, type = caption, text = sub 2", 
"3 MarkerReached: ms = 2133, type = text, text = some text here", 
"4 MarkerReached: ms = 2933, type = caption, text = sub 3", 
"5 MarkerReached: ms = 3533, type = caption, text = <center>sub 4</center>", 
"6 MarkerReached: ms = 4333, type = caption, text = <b>sub 5</b>", 
"MediaEnded"
]


tests.push ("timecode-mini-caption-all");
results ["timecode-mini-caption-all"] = [
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

tests.push ("timecode-short-caption-1");
results ["timecode-short-caption-1"] = [
"MediaOpened",
"1 MarkerReached: ms = 1004, type = caption, text = one sec",
"MediaEnded"
];

tests.push ("timecode-short-mpeg4");
results ["timecode-short-mpeg4"] = ["MediaFailed"];

tests.push ("timecode-short-vc1-adv");
results ["timecode-short-vc1-adv"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-vc1-main");
results ["timecode-short-vc1-main"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-vc1-simple");
results ["timecode-short-vc1-simple"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short");
results ["timecode-short"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv7");
results ["timecode-short-wmv7"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv8");
results ["timecode-short-wmv8"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode-short-wmv9src");
results ["timecode-short-wmv9src"] = ["MediaFailed"];

tests.push ("timecode-short-wmv9");
results ["timecode-short-wmv9"] = ["MediaOpened", "MediaEnded"];

tests.push ("timecode");
results ["timecode"] = ["MediaFailed"];


