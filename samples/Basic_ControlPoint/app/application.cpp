#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <Network/UPnP/schemas-upnp-org/device/MediaRenderer1.h>
#include <Network/UPnP/schemas-upnp-org/device/MediaServer1.h>

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

void findMediaRenderers()
{
	controlPoint.cancelSearch();
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

		auto& transport = device.getAVTransport();
		transport.action_GetDeviceCapabilities(0, [&device](auto& result) {
			Serial.print(device.caption());
			Serial.println(_F(": Device Capabilities = "));
			result.printTo(Serial);
			Serial.println(_F("---"));
			Serial.println();
		});

		// Keep this device
		return true;
	}));

	// Stop after 10 seconds
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<10000>(InterruptCallback([]() { controlPoint.cancelSearch(); })).startOnce();
}

void findMediaServers()
{
	controlPoint.cancelSearch();
	controlPoint.beginSearch(Delegate<bool(MediaServer1&)>([](auto& device) {
		// Stop at the first response
		//		controlPoint.cancelSearch();

		Serial.print(_F("Found: "));
		Serial.println(device.caption());

		auto& dir = device.getContentDirectory();

		dir.action_Browse("0", "BrowseMetadata", "*", 0, 10, nullptr, [&device](auto& result) {
			Serial.print(device.caption());
			Serial.print(_F(": Browse first 10 metadata: "));
			result.printTo(Serial);
		});

		return true;
	}));

	// Stop after 5 seconds and do a search for renderers
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<5000>(findMediaRenderers).startOnce();
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

	findMediaServers();
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
