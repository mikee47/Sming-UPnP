DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP

DEVICE_CONFIGS := $(patsubst config/%,%,$(call ListAllSubDirs,config))

XML_INCFILES := $(foreach d,$(DEVICE_CONFIGS),include/device/$d/DeviceControl.h)

App-build: $(XML_INCFILES)

.PHONY: xml
xml: ##@Generate headers from XML descriptions
	$(call PrintVariable,DEVICE_CONFIGS)
	$(Q) $(MAKE) -B $(XML_INCFILES)

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm $(XML_INCFILES)

$(XML_INCFILES):
	$(call upnp_generate_device,$(patsubst include/device/%/DeviceControl.h,config/%,$@),$(patsubst include/device/%/DeviceControl.h,ddd.xml,$@),$@)

include/device:
	$(Q) mkdir -p $@
