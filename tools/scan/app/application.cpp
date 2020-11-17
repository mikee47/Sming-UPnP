#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>

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
	controlPoint.beginSearch(UPnP::RootDeviceUrn(), [](HttpConnection& connection, XML::Document& description) {
		Serial.println();
		Serial.println();
		Serial.print(F("Remote IP: "));
		Serial.print(connection.getRemoteIp());
		Serial.print(':');
		Serial.println(connection.getRemotePort());
		Serial.println(connection.getResponse()->toString());
		XML::serialize(description, Serial, true);
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

void urnTest(UPnP::Urn::Kind kind, const String& s)
{
	UPnP::Urn urn(s);
	Serial.print("kind = ");
	Serial.println(int(urn.kind));
	Serial.println(urn.toString());
}

void test()
{
#define URN_STRING_MAP(XX)                                                                                             \
	XX(uuid, "uuid:{uuid}", "uuid:{uuid}")                                                                             \
	XX(root, "upnp:rootdevice", "uuid:{uuid}::upnp:rootdevice")                                                        \
	XX(device, "urn:{domain}:device:{deviceType}:{version}",                                                           \
	   "uuid:{uuid}::urn:{domain}:device:{deviceType}:{version}")                                                      \
	XX(service, "urn:{domain}:service:{serviceType}:{version}",                                                        \
	   "uuid:{uuid}::urn:{domain}:service:{serviceType}:{version}")

	UPnP::Urn urn;

#define XX(kind, urnString, usnString)                                                                                 \
	urnTest(UPnP::Urn::Kind::kind, urnString);                                                                         \
	urnTest(UPnP::Urn::Kind::kind, usnString);
	URN_STRING_MAP(XX)
#undef XX
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

	test();

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
