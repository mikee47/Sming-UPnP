#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <Network/UPnP/schemas-upnp-org/ClassGroup.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
UPnP::ControlPoint controlPoint;

using namespace UPnP::schemas_upnp_org::device;
using namespace UPnP::schemas_upnp_org::service;

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

bool checkResponse(UPnP::ActionResponse& response)
{
	if(auto fault = response.fault()) {
		fault.printTo(Serial);
		return false;
	}

	return true;
}

void findMediaRenderers()
{
	controlPoint.cancelSearch();
	controlPoint.beginSearch(Delegate<bool(MediaRenderer1&)>([](auto& device) {
		// Stop at the first response
		//		controlPoint.cancelSearch();

		Serial.print(_F("Found: "));
		Serial.println(device.caption());

		auto render = device.getRenderingControl();
		if(render != nullptr) {
			render->listPresets(0, [&device](auto response) {
				Serial.println();
				Serial.println(_F("render->listPresets(0):"));
				if(checkResponse(response)) {
					Serial.print(device.friendlyName());
					Serial.print(_F(": Current presets = "));
					Serial.println(response.getCurrentPresetNameList());
				}
				Serial.println();
			});

			render->getVolume(0, RenderingControl1::Channel::fs_Master, [&device](auto response) {
				Serial.println();
				Serial.println(_F("render->getVolume(0, Master):"));
				if(checkResponse(response)) {
					Serial.print(device.friendlyName());
					Serial.print(_F(": Current Volume = "));
					Serial.println(response.getCurrentVolume());
				}
				Serial.println();
			});
		}

		auto conn = device.getConnectionManager();
		if(conn != nullptr) {
			conn->getCurrentConnectionInfo(0, [&device](auto response) {
				Serial.println();
				Serial.println(_F("conn->getCurrentConnectionInfo(0):"));
				if(checkResponse(response)) {
					Serial.print(device.friendlyName());
					Serial.println(_F(": Current Connection Info = "));
					response.printTo(Serial);
					Serial.println(_F("---"));
				}
				Serial.println();
			});
		}

		auto transport = device.getAVTransport();
		if(transport != nullptr) {
			transport->getDeviceCapabilities(0, [&device](auto response) {
				Serial.println();
				Serial.println(_F("transport->getDeviceCapabilities(0):"));
				if(checkResponse(response)) {
					Serial.print(device.friendlyName());
					Serial.println(_F(": Device Capabilities = "));
					response.printTo(Serial);
					Serial.println(_F("---"));
				}
				Serial.println();
			});
		}

		// Keep this device
		return true;
	}));

	// Stop after 10 seconds
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<10000>(InterruptCallback([]() {
		controlPoint.cancelSearch();
		Serial.println();
		Serial.println(_F("OK, I'm all done for now. What shall we do next?"));
		Serial.println();
	}));
	timer->startOnce();
}

void findMediaServers()
{
	controlPoint.cancelSearch();
	controlPoint.beginSearch(Delegate<bool(MediaServer1&)>([](auto& device) {
		// Stop at the first response
		//		controlPoint.cancelSearch();

		Serial.print(_F("Found: "));
		Serial.println(device.caption());

		auto dir = device.getContentDirectory();
		if(dir != nullptr) {
			auto printBrowseResult = [&device](auto response) {
				Serial.println();
				Serial.print(_F("Browse response for "));
				Serial.print(device.friendlyName());
				Serial.println(':');
				if(checkResponse(response)) {
					XML::Document doc;
					String s = response.getResult();
					XML::deserialize(doc, s);
					XML::serialize(doc, Serial, true);
					response.printTo(Serial);
				}
				Serial.println();
			};

			dir->browse("0", ContentDirectory1::BrowseFlag::fs_BrowseMetadata, "*", 0, 10, nullptr, printBrowseResult);
			dir->browse("0", ContentDirectory1::BrowseFlag::fs_BrowseDirectChildren, "*", 0, 10, nullptr,
						printBrowseResult);

			// Send a bad request, should return a fault condition
			dir->browse(F("bad robot"), F("chicken"), "", 0, 10, nullptr, printBrowseResult);
		}

		return true;
	}));

	// Stop after 5 seconds and do a search for renderers
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<5000>(findMediaRenderers).startOnce();
}

void initUPnP()
{
	UPnP::schemas_upnp_org::registerClasses();
	findMediaServers();
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
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
