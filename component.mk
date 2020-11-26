ifeq ($(GCC_VERSION_COMPATIBLE),0)
$(error UPnP requires a recent compiler - please upgrade!)
endif

COMPONENT_DEPENDS := SSDP RapidXML

COMPONENT_PYTHON_REQUIREMENTS := requirements.txt

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools

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
	$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/$$(if $$(findstring /device/,$$<),Device,Service)$1.xsl -i $$< -o $$@
endef

# Internal function to generate source creation targets
# $1 -> Name of .xml schema, no path
# $2 -> Directory for schema
# $3 -> Directory for output source code
define upnp_generate_target
UPNP_INCFILES += $3$(1:.xml=Template.h)
$3$(1:.xml=Template.h): $2$1
	$(call upnp_generate_template,Template)

UPNP_INCFILES += $3$(1:.xml=.h)
$3$(1:.xml=.h): $2$1
	$(call upnp_generate_template,Control.hpp)

UPNP_SRCFILES += $3$(1:.xml=.cpp)
$3$(1:.xml=.cpp): $2$1
	$(call upnp_generate_template,Control.cpp)
endef

# Generate group class info target
# $1 -> Path to domain schema directory
# $2 -> Directory for output
define upnp_generate_group_target
UPNP_INCFILES += $2/ClassGroup.h
$2/ClassGroup.h:
	$$(info UPnP generate $$@)
	$(Q) $$(UPNP_TOOLS)/gen.py -t $$(UPNP_TOOLS)/xsl/ClassGroup.hpp.xsl -i $1 -o $$@

UPNP_SRCFILES += $2/ClassGroup.cpp
$2/ClassGroup.cpp:
	$(Q) $$(UPNP_TOOLS)/gen.py -t $$(UPNP_TOOLS)/xsl/ClassGroup.cpp.xsl -i $1 -o $$@
endef

# Generate all source targets from a schema directory
# $1 -> Schema directory
# $2 -> Source output directory
define upnp_generate
UPNP_SRCFILES :=
UPNP_INCFILES :=
UPNP_DESCRIPTIONS := $(call ListAllFiles,$1,*.xml)
UPNP_GROUPS := $$(sort $$(call dirx,$$(call dirx,$$(UPNP_DESCRIPTIONS))))
$$(foreach c,$$(UPNP_DESCRIPTIONS),$$(eval $$(call upnp_generate_target,$$(notdir $$c),$$(dir $$c),$2/$$(call upnp_schema_relpath,$$(dir $$c)))))
$$(foreach d,$$(UPNP_GROUPS),$$(eval $$(call upnp_generate_group_target,$$d,$2/$$(notdir $$d))))
endef

# Prevent parsing of following section more than once
ifeq (,$(UPNP_PARSED_FLAG))

# Create targets for application schema
UPNP_APP_INCDIR			:= $(abspath $(CMP_App_BUILD_BASE)/src)
$(eval $(call upnp_generate,schema,$(UPNP_APP_INCDIR)/Network/UPnP))
COMPONENT_INCDIRS		+= $(UPNP_APP_INCDIR)
COMPONENT_APPCODE		:= $(abspath $(sort $(call dirx,$(UPNP_SRCFILES))))
CMP_App_PREREQUISITES	+= upnp_app_prerequisites

.PHONY: upnp_app_prerequisites
upnp_app_prerequisites: $(UPNP_SRCFILES) $(UPNP_INCFILES)

#
# Tool for scanning, fetching and parsing. Needs to be pre-built
#
UPNP_SCAN_TOOL = $(UPNP_TOOLS)/scan/out/Host/debug/firmware/scan$(TOOL_EXT)

.PHONY: upnp-tools
upnp-tools: ##Build UPnP tools - append SMING_ARCH as required
	$(Q) $(MAKE) -C $(UPNP_TOOLS)/scan

$(UPNP_SCAN_TOOL):
	$(info )
	$(error Please run "make upnp-tools SMING_ARCH=Host")

#
UPNP_SCAN = $(UPNP_SCAN_TOOL) $(HOST_NETWORK_OPTIONS) -- scan $(HOST_PARAMETERS)
UPNP_PARSE = $(UPNP_SCAN_TOOL) --nonet -- parse $(HOST_PARAMETERS)
UPNP_FETCH = $(UPNP_SCAN_TOOL) --nonet fetch $(HOST_PARAMETERS)

.PHONY: upnp-check-scan

.PHONY: upnp-scan
upnp-scan: $(UPNP_SCAN_TOOL) ##Run network scan (use HOST_PARAMETERS)
	$(UPNP_SCAN)

.PHONY: upnp-parse
upnp-parse: $(UPNP_SCAN_TOOL) ##Parse raw device descriptions (use HOST_PARAMETERS)
	$(UPNP_PARSE)

.PHONY: upnp-fetch
upnp-fetch: $(UPNP_SCAN_TOOL) ##Fetch device description by URL (use HOST_PARAMETERS)
	$(UPNP_FETCH)

UPNP_PARSED_FLAG := X
endif
