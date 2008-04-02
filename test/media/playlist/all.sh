#!/bin/bash -e

DRTLIST=drtlist.xml
echo "files = [" > all.js
echo "<DRTList>" > $DRTLIST
I=1
ls *.asx -1 | while read file; do
	BASENAME=`basename $file .asx`
	echo "\"${file}\"," >> all.js
	G=`echo ${file} | sed 's/\.asx//'`.g.html
	cp all-data.template.html $G
	cat "$G" | sed "s/ASX_FILE/${file}/" > tmpfile
	cat tmpfile > "$G"
	echo -n "    <Test id=\"${I}\" inputFile=\"${BASENAME}.g.html\" masterFile10=\"None\" timeout=\"10000\" categories=\"playlist,media\" " >> $DRTLIST

	cat ignore.txt | while read ignore; do
		OLDIFS=$IFS
		IFS="|"
		declare -a array
		array=( $ignore)
		if [ "x${array[0]}" == "x$BASENAME" ]; then
			echo -n " ignore=\"True\" ignoreReason=\"${array[1]}\" " >> $DRTLIST
		fi
		IFS=$OLDIFS
	done

	echo " />" >> $DRTLIST
	let I=$I+1
done
rm tmpfile
echo "]" >> all.js
echo "</DRTList>" >> $DRTLIST
