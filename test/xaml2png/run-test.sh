#!/bin/bash

mode=$1
tst=$2
resultprefix=$3

if [ x$mode != "xbaseline" -a x$mode != "xtest" ] ; then
    echo unknown mode: $mode
    exit 1
fi

resultprefix=`basename $tst`
resultpng=results/$resultprefix.png
basepng=baselines/$resultprefix.png
differencespng=differences-$resultprefix.png

if test x$mode == "xbaseline"; then
    echo -n "    Generating baseline for test $tst... "

    ./xaml2png $tst $basepng

    echo done.
elif test x$mode == "xtest"; then
    echo -n "    Running test $tst... "

    rm -f $resultpng $differencespng

    ./xaml2png $tst $resultpng

    # and compare to our baseline
    if diff $resultpng $basepng >/dev/null; then
	echo PASSED.
	rm -f $resultpng
    else
	echo FAILED.
	convert $resultpng $basepng -compose difference -composite $differencespng
    fi
fi
