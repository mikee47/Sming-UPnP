#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <upnp/device/Panasonic/42AS500_Series/MediaRenderer1.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
UPnP::ControlPoint controlPoint(8192);

using MediaRenderer1 = UPnP::schemas_upnp_org::device::MediaRenderer1;

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void initUPnP()
{
	controlPoint.beginSearch(Delegate<bool(MediaRenderer1&)>([](auto& device) {
		// Stop at the first response
		//		controlPoint.cancelSearch();

		Serial.print(_F("Found: "));
		Serial.println(device.caption());

		auto& render = device.getRenderingControl();

		render.action_ListPresets(0, [&device](auto& result) {
			Serial.print(device.caption());
			Serial.print(_F(": Current presets = "));
			Serial.println(result.vCurrentPresetNameList);
		});

		render.action_GetVolume(0, nullptr, [&device](auto& result) {
			Serial.print(device.caption());
			Serial.print(_F(": Current Volume = "));
			Serial.println(result.vCurrentVolume);
		});

		auto& conn = device.getConnectionManager();

		conn.action_GetCurrentConnectionInfo(0, [&device](auto& result) {
			Serial.print(device.caption());
			Serial.println(_F(": Current Connection Info = "));
			result.printTo(Serial);
			Serial.println(_F("---"));
			Serial.println();
		});

		// Keep this device
		return true;
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
