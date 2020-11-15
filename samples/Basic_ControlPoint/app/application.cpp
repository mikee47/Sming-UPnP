#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <device/dmr.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
UPnP::ControlPoint controlPoint;
DeviceClass_MediaRenderer mediaRenderer;

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void initUPnP()
{
	controlPoint.beginSearch(
		mediaRenderer.RenderingControl, [](UPnP::DeviceControl* device, UPnP::ServiceControl* service) {
			Serial.print("Found: ");
			Serial.println(device->friendlyName());

			if(service == nullptr) {
				debug_e("Service %s missing from %s",
						mediaRenderer.RenderingControl.getField(UPnP::Service::Field::serviceType).c_str(),
						device->deviceType().c_str());
			}

			delete device;
		});
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	debugf("GotIP: %s", ip.toString().c_str());

	if(ntpClient == nullptr) {
		ntpClient = new NtpClient([](NtpClient& client, time_t timestamp) {
			SystemClock.setTime(timestamp, eTZ_UTC);
			Serial.print("Time synchronized: ");
			Serial.println(SystemClock.getSystemTimeString());
		});
	};

	initUPnP();
}

} // namespace

void init()
{
	Serial.setTxBufferSize(4096);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
