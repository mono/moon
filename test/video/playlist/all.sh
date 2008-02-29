echo "files = [" > all.js
ls *.asx -1 | while read file; do
	echo "\"${file}\"," >> all.js
done
echo "]" >> all.js
