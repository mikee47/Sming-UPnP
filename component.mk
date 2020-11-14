COMPONENT_DEPENDS := SSDP RapidXML
COMPONENT_INCDIRS := src/include
COMPONENT_SRCDIRS := src

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools

UPNP_GENERATE_DEVICE = $(python) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/device.xsl
