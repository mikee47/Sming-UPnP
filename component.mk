COMPONENT_DEPENDS := SSDP RapidXML

COMPONENT_PYTHON_REQUIREMENTS := requirements.txt

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools
UPNP_SCHEMA		:= $(COMPONENT_PATH)/schema
UPNP_INCDIR		:= $(COMPONENT_BUILD_BASE)/src/Network/UPnP

COMPONENT_INCDIRS := \
	src/include \
	$(COMPONENT_BUILD_BASE)/src

#
# $1 -> Template
# $2 -> Source .xml file
# $3 -> Output .h file
define upnp_generate_template
$(info UPnP generate $1)
$(info --> "$2")
$(info <-- "$3")
$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$1.hpp.xsl -i $2 -o $3
$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$1.cpp.xsl -i $2 -o $(3:.h=.cpp)
endef

#
# $1 -> Source .xml file path
# $2 -> Output .h file
define upnp_generate
$(call upnp_generate_template,$(if $(findstring /device/,$1),DeviceControl,ServiceControl),$1,$2)
endef

#
UPNP_CONFIGS := $(patsubst $(UPNP_SCHEMA)/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,$(UPNP_SCHEMA)),$d/*.xml)))
UPNP_INCFILES := $(foreach f,$(UPNP_CONFIGS),$(UPNP_INCDIR)/$(f:.xml=.h))

COMPONENT_SRCDIRS := \
	src \
	$(sort $(foreach f,$(UPNP_INCFILES),$(dir $f)))

COMPONENT_PREREQUISITES := $(UPNP_INCFILES)

$(UPNP_INCFILES):
	$(call upnp_generate,$(patsubst $(UPNP_INCDIR)/%,$(UPNP_SCHEMA)/%,$(@:.h=.xml)),$@)

.PHONY: upnp-schema
upnp-schema: ##Generate source files from XML descriptions
	$(Q) $(MAKE) -B $(UPNP_INCFILES)

UPnP-clean: upnp-schema-clean

.PHONY: upnp-schema-clean
upnp-schema-clean: ##Clean UPnP generated source files
	-$(Q) rm -rf $(UPNP_INCDIR)

