if ONLY_BUILD_MONO
all all-local:
	$(MAKE) -C $(MCS_PATH)/../mono moon-do-build

clean clean-local:
	$(MAKE) -C $(MCS_PATH)/../mono moon-do-clean

else
all all-local clean clean-local:
	$(MAKE) -C $(MCS_PATH)/../mono $@

endif

install:

