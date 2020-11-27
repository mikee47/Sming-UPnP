DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP

DEVICE_SCHEMADIR	:= schema/device
DEVICE_INCDIR		:= out/Network/UPnP/device
DEVICE_CONFIGS		= $(patsubst $(DEVICE_SCHEMADIR)/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,$(DEVICE_SCHEMADIR)),$d/*.xml)))
DEVICE_INCFILES		= $(foreach f,$(DEVICE_CONFIGS),$(DEVICE_INCDIR)/$(f:.xml=.h))

SERVICE_SCHEMADIR	= schema/service
SERVICE_INCDIR		= out/Network/UPnP/service
SERVICE_CONFIGS		= $(patsubst $(SERVICE_SCHEMADIR)/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,$(SERVICE_SCHEMADIR)),$d/*.xml)))
SERVICE_INCFILES	= $(foreach f,$(SERVICE_CONFIGS),$(SERVICE_INCDIR)/$(f:.xml=.h))

COMPONENT_INCDIRS	:= include out
COMPONENT_SRCDIRS	:= app $(sort $(foreach f,$(DEVICE_INCFILES) $(SERVICE_INCFILES),$(dir $f)))

App-build: $(DEVICE_INCFILES) $(SERVICE_INCFILES)

.PHONY: xml
xml: ##@Generate headers from XML descriptions
	$(Q) $(MAKE) -B $(DEVICE_INCFILES) $(SERVICE_INCFILES)

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm -rf $(UPNP_INCDIR)

$(DEVICE_INCFILES):
	$(call upnp_generate_device,$(patsubst $(DEVICE_INCDIR)/%,$(DEVICE_SCHEMADIR)/%,$(@:.h=.xml)),$@)

$(SERVICE_INCFILES):
	$(call upnp_generate_service,$(patsubst $(SERVICE_INCDIR)/%,$(SERVICE_SCHEMADIR)/%,$(@:.h=.xml)),$@)
