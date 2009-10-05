#!/bin/bash

if [ "x$1" == "x" ]
then
	echo "Usage $0 novell-moonlight.xpi"
	exit 1
fi

if [ ! -f $1 ]
then
	echo "$1 is not a regular file"
	exit 1
fi

DIR=$(mktemp -d -p .)

unzip -d $DIR $1 > /dev/null

echo "Checking $1"

STATUS=0
BADLIB="xcb"  # this lib should never be linked to
cd $DIR

for SOFILE in $(find . -name *.so)
do
	#echo "Checking $SOFILE"
	objdump -p $SOFILE | grep NEEDED | grep -i $BADLIB > /dev/null
	if [ $? == 0 ]
	then

		echo "ERROR: $SOFILE is linked to $BADLIB"
		#objdump -p $SOFILE | grep NEEDED
		STATUS=1
	fi
done

rm -rf $DIR
exit $STATUS
