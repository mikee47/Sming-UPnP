COMPONENT_DEPENDS := SSDP RapidXML

COMPONENT_PYTHON_REQUIREMENTS := requirements.txt

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools

# Application can set these in their component.mk file (which is always parsed first)
UPNP_APP_SCHEMA	?=
UPNP_APP_INCDIR	?= $(CMP_App_BUILD_BASE)/src/Network/UPnP

COMPONENT_INCDIRS := \
	src/include \
	$(COMPONENT_BUILD_BASE)/src

COMPONENT_SRCDIRS := src

# Get base schema directory. For example:
#	/home/user/Sming/Sming/Libraries/UPnP/schema/schemas-upnp-org/device/MediaServer1.xml
# gives:
#	/home/user/Sming/Sming/Libraries/UPnP/schema/
upnp_schema_dir = $(call dirx,$(call dirx,$(call dirx,$1)))

# Get schema relative to base schema directory. For example:
#	/home/user/Sming/Sming/Libraries/UPnP/schema/schemas-upnp-org/device/MediaServer1.xml
# gives:
#	schemas-upnp-org/device/MediaServer1.xml
upnp_schema_relpath = $(patsubst $(call upnp_schema_dir,$1)/%,%,$1)

# Generate header or source for a device or service schema
# $< -> Source .xml file path
# $@ -> Output file
# $1 -> Template kind (hpp or cpp)
define upnp_generate_template
	$$(info UPnP generate $$(<F) -> $$(@F))
	$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$$(if $$(findstring /device/,$$<),DeviceControl,ServiceControl).$1.xsl -i $$< -o $$@
endef

# Internal function to generate source creation targets
# $1 -> Name of .xml schema, no path
# $2 -> Directory for schema
# $3 -> Directory for output source code
define upnp_generate_target
UPNP_INCFILES += $3$(1:.xml=.h)
$3$(1:.xml=.h): $2$1
	$(call upnp_generate_template,hpp)

UPNP_SRCFILES += $3$(1:.xml=.cpp)
$3$(1:.xml=.cpp): $2$1
	$(call upnp_generate_template,cpp)
endef

# Generate all source targets from a schema directory
# $1 -> Schema directory
# $2 -> Source output directory
define upnp_generate
UPNP_SRCFILES :=
UPNP_INCFILES :=
$$(foreach c,$$(call ListAllFiles,$1,*.xml),$$(eval $$(call upnp_generate_target,$$(notdir $$c),$$(dir $$c),$2/$$(call upnp_schema_relpath,$$(dir $$c)))))
endef

# If specified, create targets for application source generation
ifneq (,$(UPNP_APP_SCHEMA))
ifeq (,$(UPNP_APP_SCHEMA_FLAG))
$(eval $(call upnp_generate,$(UPNP_APP_SCHEMA),$(UPNP_APP_INCDIR)))
COMPONENT_APPCODE := $(abspath $(sort $(call dirx,$(UPNP_SRCFILES))))
CMP_App_PREREQUISITES += upnp_app_prerequisites 

.PHONY: upnp_app_prerequisites
upnp_app_prerequisites: $(UPNP_SRCFILES) $(UPNP_INCFILES)

UPNP_APP_SCHEMA_FLAG := X
endif
endif

#
# Tool for scanning, fetching and parsing. Needs to be pre-built
#
UPNP_SCAN_TOOL = $(UPNP_TOOLS)/scan/out/Host/debug/firmware/scan$(TOOL_EXT)

.PHONY: upnp-tools
upnp-tools: $(UPNP_SCAN_TOOL) ##Build UPnP tools - append SMING_ARCH as required

$(UPNP_SCAN_TOOL):
	$(Q) $(MAKE) -C $(UPNP_TOOLS)/scan -e 

#
UPNP_SCAN = $(UPNP_SCAN_TOOL) --nonet scan
UPNP_PARSE = $(UPNP_SCAN_TOOL) --nonet parse
UPNP_FETCH = $(UPNP_SCAN_TOOL) --nonet fetch
