DATA_XAP=Controls.Data.Test.xap
DATA_XAP_FILE=$(srcdir)/../Controls.Data.Test/$(DATA_XAP)
EXTENDED_XAP=ControlsExtended.Test.xap
EXTENDED_XAP_FILE=$(srcdir)/../Controls.Extended.Test/$(EXTENDED_XAP)

CLEANFILES = 
	$(DATA_XAP)	\
	$(EXTENDED_XAP)

$(DATA_XAP): $(DATA_XAP_FILE)
	-cp $(DATA_XAP_FILE) .

$(EXTENDED_XAP): $(EXTENDED_XAP_FILE)
	-cp $(EXTENDED_XAP_FILE) .


test: $(EXTENDED_XAP) $(DATA_XAP) 
