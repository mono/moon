for file in *.xaml
do
	echo trying $file
	mopen --timeout 1 $file
done
