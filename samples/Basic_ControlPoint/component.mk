DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP

DEVNAMES		:= dmr dms nrc
XML_INCFILES 	:= $(foreach d,$(DEVNAMES),include/device/$d.h)

App-build: $(XML_INCFILES)

.PHONY: xml
xml: ##@Generate headers from XML descriptions
	$(Q) $(MAKE) -B $(XML_INCFILES) include/device/hg1.h

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm $(XML_INCFILES)

$(XML_INCFILES): | include/device
	$(call upnp_generate_device,config/viera,$(@F:.h=)/ddd.xml,$@)

include/device/hg1.h:
	$(call upnp_generate_device,config/hg1,ddd.xml,$@)


include/device:
	$(Q) mkdir -p $@
