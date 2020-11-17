COMPONENT_DEPENDS := SSDP RapidXML
COMPONENT_INCDIRS := src/include
COMPONENT_SRCDIRS := src

COMPONENT_PYTHON_REQUIREMENTS := requirements.txt

UPNP_TOOLS		:= $(COMPONENT_PATH)/tools

#
# $1 -> Base directory
# $2 -> Source .xml file path relative to base
# $3 -> Output .h file
define upnp_generate_device
@echo Generating $3
$(Q) $(UPNP_TOOLS)/gen.py -t $(UPNP_TOOLS)/xsl/DeviceControl.xsl -s $1 -i $2 -o $3
endef
