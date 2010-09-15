if HAVE_GALLIUM
all all-local:
	$(MAKE) -C $(GALLIUM_PATH)/src/gallium
	$(MAKE) -C $(GALLIUM_PATH)/src/gallium/winsys

clean clean-local:
	$(MAKE) -C $(GALLIUM_PATH)/src/gallium clean
	$(MAKE) -C $(GALLIUM_PATH)/src/gallium/winsys clean

install:

endif