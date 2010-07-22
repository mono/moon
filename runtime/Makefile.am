if ONLY_BUILD_MONO
all all-local:
	$(MAKE) -C $(MONO_PATH) moon-do-build

clean clean-local:
	$(MAKE) -C $(MONO_PATH) moon-do-clean

else
all all-local clean clean-local:
	$(MAKE) -C $(MONO_PATH) $@

endif

install:

