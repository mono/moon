default:
	cp site/dependency_propertiesTestPage.html index.html
	sed 's/ClientBin\///' -i index.html
	mxap --generate-html- --entry-point-type=dependency_properties.App

test: default
	firefox *.html
