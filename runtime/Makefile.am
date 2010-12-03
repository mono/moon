if ONLY_BUILD_MONO
all all-local:
	$(MAKE) -C $(MONO_PATH) moon-do-build

clean clean-local:
	$(MAKE) -C $(MONO_PATH) moon-do-clean

else
all all-local:
	$(MAKE) -C $(MONO_PATH) $@
	$(MAKE) -C $(MONO_PATH)/runtime moon-do-basic

clean clean-local:
	$(MAKE) -C $(MONO_PATH) $@
	$(MAKE) -C $(MONO_PATH)/runtime moon-do-basic-clean

endif

install:

