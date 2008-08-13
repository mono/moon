default:
	cp site/dependency_propertiesTestPage.html dependency_properties.html
	sed 's/ClientBin\///' -i dependency_properties.html
	mxap --generate-html-

test: default
	firefox *.html
