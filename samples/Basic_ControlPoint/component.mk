DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP

DEVNAMES		:= dmr dms nrc
XML_INCFILES 	:= $(foreach d,$(DEVNAMES),include/device/$d.h)

App-build: $(XML_INCFILES)

.PHONY: xml
xml: ##@Generate headers from XML descriptions
	$(Q) $(MAKE) -B $(XML_INCFILES)

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm $(XML_INCFILES)

$(XML_INCFILES): | include/device
	@echo Generating $@
	$(Q) $(UPNP_GENERATE_DEVICE) -i config/$(@F:.h=)/ddd.xml -o $@

include/device:
	$(Q) mkdir -p $@
