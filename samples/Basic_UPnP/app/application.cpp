#include <SmingCore.h>
#include <Network/UPnP/DeviceHost.h>
#include <Network/UPnP/ControlPoint.h>
#include <Network/SSDP/Server.h>
#include <TeaPot.h>
#include <Wemo.h>
#include <malloc_count.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
HttpServer server;
TeaPot teapot(1);
Wemo::Controllee wemo1(1, "Socket #1");
Wemo::Controllee wemo2(2, "Socket #2");
UPnP::ControlPoint controlPoint;

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

int onHttpRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	// Pass the request into the UPnP stack
	if(UPnP::deviceHost.onHttpRequest(connection)) {
		return 0;
	}

	// Not a UPnP request. Handle any application-specific pages here

	auto path = request.uri.getRelativePath();
	if(path.length() == 0 || path == F("index.html")) {
		auto stream = UPnP::deviceHost.generateDebugPage(F("Basic UPnP"));
		response.sendDataStream(stream, MIME_HTML);
		return 0;
	}

	Serial.print("Page not found: ");
	Serial.println(request.uri.Path);

	response.code = HTTP_STATUS_NOT_FOUND;
	return 0;
}

void simpleSearch()
{
	ServiceUrn urn("dial-multiscreen-org", "dial", 1);
	controlPoint.beginSearch(urn, [](HttpConnection& connection, XML::Document& description) {
		debug_e("Found service!");
		auto node = XML::getNode(description, F("/device/friendlyName"));
		if(node == nullptr) {
			Serial.println(_F("UNEXPECTED! friendlyName missing from device description"));
		} else {
			Serial.print(_F("Friendly name '"));
			Serial.print(node->value());
			Serial.println('\'');
		}

		// Print the response headers
		auto& headers = connection.getResponse()->headers;
		for(unsigned i = 0; i < headers.count(); ++i) {
			Serial.print(headers[i]);
		}
	});
}

void initUPnP()
{
	// Configure our HTTP Server to listen for HTTP requests
	server.listen(80);
	server.paths.setDefault(onHttpRequest);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	server.setBodyParser(MIME_XML, bodyToStringParser);

	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return;
	}

	// Advertise our Tea Pot
	UPnP::deviceHost.registerDevice(&teapot);

	// These two devices handle service requests
	UPnP::deviceHost.registerDevice(&wemo1);
	UPnP::deviceHost.registerDevice(&wemo2);

	// Simple search for devices
	simpleSearch();
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

	auto timer = new SimpleTimer;
	timer
		->initializeMs<5000>([]() {
			Serial.print("Free heap: ");
			Serial.print(system_get_free_heap_size());
			Serial.print(", used: ");
			Serial.print(MallocCount::getCurrent());
			Serial.print(", peak: ");
			Serial.println(MallocCount::getPeak());
			MallocCount::resetPeak();
		})
		.start();
}
