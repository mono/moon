#!/bin/bash -e

#
# to regenerate the js and drtlist, just run this file
# the drtlist will be written into drtlistSnippet.xml, c&p into the real drtlist.xml.
#

DRTLIST=drtlistSnippet.xml
echo "files = [" > all.js
# playlist tests: 600 - 999
I=600
rm -f $DRTLIST
ls *.asx -1 | sort | while read file; do
	BASENAME=`basename $file .asx`
	echo "\"${file}\"," >> all.js
	#G=`echo ${file} | sed 's/\.asx//'`.g.html
	#cp all-data.template.html $G
	#cat "$G" | sed "s/ASX_FILE/${file}/" > tmpfile
	#cat tmpfile > "$G"
	echo -e "\t\t\t<!-- Generated with moon/test/media/playlist/all.sh -->" >> $DRTLIST
	echo -e "\t\t\t<TestDefinition ID=\"${I}\" >" >> $DRTLIST
	echo -e "\t\t\t\t<Parameter Name=\"Title\" Value=\"$BASENAME\" />" >> $DRTLIST
	echo -e "\t\t\t\t<Parameter Name=\"HostType\" Value=\"{SupportedHostTypes.Http}\" />" >> $DRTLIST
	echo -e "\t\t\t\t<Parameter Name=\"InputFile\" Value=\"media/playlist/test.html?$BASENAME.asx\" />" >> $DRTLIST
	if [[ "x$BASENAME" == "xcase-mismatched-asx-tag" ]]; then
		echo -e "\t\t\t\t<Parameters Name=\"Moonlight\" >" >> $DRTLIST
		echo -e "\t\t\t\t\t<Parameter Name=\"KnownFailure\" Value=\"expat can't regonize tags case-insensitively\" />" >> $DRTLIST
		echo -e "\t\t\t\t</Parameters>" >> $DRTLIST
	elif [[ "x$BASENAME" == "xasx-whitespace" ]]; then
		echo -e "\t\t\t\t<Parameters Name=\"Moonlight\" >" >> $DRTLIST
		echo -e "\t\t\t\t\t<Parameter Name=\"KnownFailure\" Value=\"no idea why SL throws an AG_E_NETWORK_ERROR instead of AG_INVALID_FILE_FORMAT\" />" >> $DRTLIST
		echo -e "\t\t\t\t</Parameters>" >> $DRTLIST
	elif [[ "x$BASENAME" == "xrepeat-with-number" ]]; then
		echo -e "\t\t\t\t<Parameters Name=\"Moonlight\" >" >> $DRTLIST
		echo -e "\t\t\t\t\t<Parameter Name=\"KnownFailure\" Value=\"&lt;jackson&gt; This repeat-with-number one is going to be pretty much impossible to fix without writing a parser from scratch\n&lt;jackson&gt; expat just can't handle the missing quotes and completely misreports it\" />" >> $DRTLIST
		echo -e "\t\t\t\t</Parameters>" >> $DRTLIST
	fi
	echo -e "\t\t\t</TestDefinition>" >> $DRTLIST
	let I=$I+1
done
#rm tmpfile
echo "]" >> all.js
#echo "</DRTList>" >> $DRTLIST
