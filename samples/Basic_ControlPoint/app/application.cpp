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
UPnP::ControlPoint controlPoint(8192);

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void initUPnP()
{
	controlPoint.beginSearch(Delegate<void(Device_MediaRenderer*)>([](Device_MediaRenderer* device) {
		// Stop at the first response
		controlPoint.cancelSearch();

		Serial.print(F("Found: "));
		Serial.println(device->friendlyName());

		auto service = device->getRenderingControl();

		service.action_GetVolume(
			[](Service_RenderingControl::Volume CurrentVolume) {
				Serial.print("Current Volume: ");
				Serial.println(CurrentVolume);
			},
			0, 0);

		delete device;
	}));
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
