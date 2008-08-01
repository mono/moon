default:
	$(MAKE) install -C ../../../class
	$(MAKE) install -C ../../../src
	$(MAKE) install -C ../../../plugin
	cp site/dependency_propertiesTestPage.html dependency_properties.html
	sed 's/ClientBin\///' -i dependency_properties.html
	mxap --generate-html=no

test: default
	firefox *.html
