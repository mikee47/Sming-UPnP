COMPONENT_DEPENDS := SSDP RapidXML

COMPONENT_PYTHON_REQUIREMENTS := requirements.txt

# Add this to main Sming build ?
COMMON_SRCDIR	:= $(SMING_HOME)/out/common
# ?

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools
UPNP_SCHEMA		:= $(COMPONENT_PATH)/schema
UPNP_INCDIR		:= $(COMMON_SRCDIR)/Network/UPnP

COMPONENT_INCDIRS := \
	src/include \
	$(COMMON_SRCDIR)

#
# $1 -> Template
# $2 -> Source .xml file
# $3 -> Output .h file
define upnp_generate
$(info UPnP generate $1)
$(info --> "$2")
$(info <-- "$3")
$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$1.hpp.xsl -i $2 -o $3
$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$1.cpp.xsl -i $2 -o $(3:.h=.cpp)
endef

#
# $1 -> Source .xml file path relative to base
# $2 -> Output .h file
define upnp_generate_device
$(call upnp_generate,DeviceControl,$1,$2)
endef

#
# $1 -> Source .xml file path relative to base
# $2 -> Output .h file
define upnp_generate_service
$(call upnp_generate,ServiceControl,$1,$2)
endef

UPNP_DEVICE_SCHEMA		:= $(UPNP_SCHEMA)/device
UPNP_DEVICE_INCDIR		:= $(UPNP_INCDIR)/device
UPNP_DEVICE_CONFIGS		= $(patsubst $(UPNP_DEVICE_SCHEMA)/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,$(UPNP_DEVICE_SCHEMA)),$d/*.xml)))
UPNP_DEVICE_INCFILES	= $(foreach f,$(UPNP_DEVICE_CONFIGS),$(UPNP_DEVICE_INCDIR)/$(f:.xml=.h))

UPNP_SERVICE_SCHEMA		= $(UPNP_SCHEMA)/service
UPNP_SERVICE_INCDIR		= $(UPNP_INCDIR)/service
UPNP_SERVICE_CONFIGS	= $(patsubst $(UPNP_SERVICE_SCHEMA)/%,%,$(wildcard $(foreach d,$(call ListAllSubDirs,$(UPNP_SERVICE_SCHEMA)),$d/*.xml)))
UPNP_SERVICE_INCFILES	= $(foreach f,$(UPNP_SERVICE_CONFIGS),$(UPNP_SERVICE_INCDIR)/$(f:.xml=.h))

COMPONENT_SRCDIRS := \
	src \
	$(sort $(foreach f,$(UPNP_DEVICE_INCFILES) $(UPNP_SERVICE_INCFILES),$(dir $f)))
	
COMPONENT_TARGETS += $(UPNP_DEVICE_INCFILES) $(UPNP_SERVICE_INCFILES)

ifeq (,$(COMPONENT_RULE))

$(UPNP_DEVICE_INCFILES):
	$(call upnp_generate_device,$(patsubst $(UPNP_DEVICE_INCDIR)/%,$(UPNP_DEVICE_SCHEMA)/%,$(@:.h=.xml)),$@)

$(UPNP_SERVICE_INCFILES):
	$(call upnp_generate_service,$(patsubst $(UPNP_SERVICE_INCDIR)/%,$(UPNP_SERVICE_SCHEMA)/%,$(@:.h=.xml)),$@)

endif


.PHONY: upnp-schema
upnp-schema: ##Generate source files from XML descriptions
	$(Q) $(MAKE) -B $(UPNP_DEVICE_INCFILES) $(UPNP_SERVICE_INCFILES)

UPnP-clean: upnp-schema-clean

.PHONY: upnp-schema-clean
upnp-schema-clean: ##Clean UPnP generated source files
	-$(Q) rm -rf $(UPNP_INCDIR)

