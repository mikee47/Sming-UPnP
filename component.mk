ifeq ($(GCC_VERSION_COMPATIBLE),0)
$(error UPnP requires a recent compiler - please upgrade!)
endif

COMPONENT_DEPENDS := SSDP RapidXML
UPNP_TOOLS := $(COMPONENT_PATH)/tools

COMPONENT_INCDIRS := \
	src/include \
	$(COMPONENT_BUILD_BASE)/src

COMPONENT_SRCDIRS := src

# Create targets for application schema
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
