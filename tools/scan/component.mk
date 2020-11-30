DISABLE_SPIFFS := 1
ARDUINO_LIBRARIES := UPnP
APP_NAME := scan

DEVICE_DIR := out/devices
SCHEMA_DIR := out/upnp

clean: xml-clean

.PHONY: xml-clean
xml-clean:
	-$(Q) rm -rf $(DEVICE_DIR) $(SCHEMA_DIR)
