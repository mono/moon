default:
	mxap --generate-html- --entry-point-type=dependency_properties.App
	cp *.xap site/ClientBin

test: default
	firefox site/index.html
