ASF_TESTS = *.wmv

asf:
	cd ../../src/asf/ && make

test-asf: asf
	@for t in $(ASF_TESTS); do \
		../../src/asf/asf -test:$$t; \
	done
	
