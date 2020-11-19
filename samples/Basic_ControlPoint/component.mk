DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP

APP_INCDIR		:= out/upnp
APP_CONFIGS		:= $(patsubst schema/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,schema),$d/*.xml)))
APP_INCFILES	:= $(foreach f,$(APP_CONFIGS),$(APP_INCDIR)/$(f:.xml=.h))

COMPONENT_INCDIRS	:= include out
COMPONENT_SRCDIRS	:= app $(sort $(foreach f,$(APP_INCFILES),$(dir $f)))

App-build: $(APP_INCFILES)

.PHONY: xml
xml: ##@Generate headers from XML descriptions
	$(Q) $(MAKE) -B $(APP_INCFILES)

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm -rf $(UPNP_INCDIR)

$(APP_INCFILES):
	$(call upnp_generate,$(patsubst $(APP_INCDIR)/%,schema/%,$(@:.h=.xml)),$@)
