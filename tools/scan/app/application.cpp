#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <io.h>
#include <Data/Stream/HostFileStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
UPnP::ControlPoint controlPoint(8192);
Timer timer;

// Fetch one description file at a time to avoid swamping the TCP stack
struct Fetch {
	String url;
	String path;

	bool operator==(const Fetch& other)
	{
		return path == other.path;
	}
};

Vector<String> ssdpDoneList;
Vector<Fetch> descriptionFetchList;
Vector<Fetch> descriptionDoneList;
Vector<String> urnFetchList;
Vector<String> urnDoneList;

static const char* baseOutputDir = "devices";

IMPORT_FSTR(gatedesc_xml, PROJECT_DIR "/devices1/192.168.1.254/1900/gatedesc.xml")

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void makedirs(const String& path)
{
	String buf = path;
	buf.replace('\\', '/');
	char* s = buf.begin();
	char* p = s;
	while((p = strchr(p, '/')) != nullptr) {
		*p = '\0';
		mkdir(s);
		*p++ = '/';
	}
}

void checkPath(String& path)
{
	path.replace(':', '-');
	path.replace('?', '-');
	path.replace('&', '-');
}

void parseDescription(XML::Document& doc)
{
	// TODO
}

void fetchNextDescription()
{
	if(descriptionFetchList.count() == 0) {
		return;
	}

	Fetch f = descriptionFetchList.at(0);
	auto path = f.path;

	Serial.print(F("Fetching '"));
	Serial.print(f.url);
	Serial.print(F("' to '"));
	Serial.print(f.path);
	Serial.println('\'');

	controlPoint.requestDescription(f.url, [path](HttpConnection& connection, XML::Document* description) {
#if DEBUG_VERBOSE_LEVEL == DBG
		Serial.println();
		Serial.println();
		Serial.println(F("====== BEGIN ======"));
		Serial.print(F("Remote IP: "));
		Serial.print(connection.getRemoteIp());
		Serial.print(':');
		Serial.println(connection.getRemotePort());

		Serial.println(F("Request: "));
		Serial.println(connection.getRequest()->toString());
		Serial.println();

		Serial.println(connection.getResponse()->toString());
		Serial.println();

		if(description != nullptr) {
			Serial.println(F("Content:"));
			XML::serialize(*description, Serial, true);
			Serial.println();
			Serial.println(F("======  END  ======"));
		}
#endif

		Fetch f = descriptionFetchList.at(0);
		descriptionFetchList.remove(0);

		if(description == nullptr) {
			// Fetch failed, move to end of queue
			Serial.print(F("Fetch '"));
			Serial.print(f.url);
			Serial.println("' failed, re-queueing");
			descriptionFetchList.add(f);
		} else {
			// Write description
			Serial.print(F("Writing "));
			Serial.println(path);
			makedirs(path);
			HostFileStream fs(path, eFO_CreateNewAlways | eFO_WriteOnly);
			XML::serialize(*description, fs, true);

			descriptionDoneList.add(f);

			parseDescription(*description);
		}

		fetchNextDescription();
	});

	//	timer.initializeMs<5000>(fetchNextDescription).startOnce();
}

void onSsdp(SSDP::BasicMessage& msg)
{
	String location = msg[HTTP_HEADER_LOCATION];
	Serial.print("Location: ");
	Serial.println(location);

	if(ssdpDoneList.indexOf(location) >= 0) {
		Serial.print(F("Ignoring, already processed"));
		return;
	}
	ssdpDoneList.add(location);

	// Create root directory for device
	String rootDir = baseOutputDir;
	rootDir += '/';
	Url url(location);
	rootDir += url.Host;
	rootDir += '/';
	rootDir += url.Port;
	rootDir += '/';
	String path = url.getRelativePath();
	int sep = path.lastIndexOf('/');
	if(sep >= 0) {
		rootDir.concat(path.c_str(), sep);
		rootDir += '/';
	}
	makedirs(rootDir);

	// Write SSDP response
	auto devtype = msg["ST"];
	if(devtype == nullptr) {
		devtype = msg["NT"];
	}
	path = rootDir + "ssdp-" + devtype + ".txt";
	checkPath(path);
	Serial.print(F("Writing "));
	Serial.println(path);
	HostFileStream fs(path, eFO_CreateNewAlways | eFO_WriteOnly);
	fs.print(F("Message type: "));
	fs.println(toString(msg.type));
	fs.println();
	for(unsigned i = 0; i < msg.count(); ++i) {
		fs.print(msg[i]);
	}

	path = rootDir + url.getRelativePath();
	checkPath(path);
	Fetch f{location, path};
	if(descriptionFetchList.contains(f)) {
		Serial.print(path);
		Serial.println(F(" already queued"));
		return;
	}
	if(descriptionDoneList.contains(f)) {
		Serial.print(path);
		Serial.println(F(" already fetched"));
		return;
	}

	descriptionFetchList.add(f);

	if(descriptionFetchList.count() == 1) {
		fetchNextDescription();
	} else {
		Serial.print(F("Queuing '"));
		Serial.print(location);
		Serial.print(F("', "));
		Serial.print(descriptionFetchList.count());
		Serial.println(F(" descriptions queued"));
	}
}

/*

Scanning starts with root devices.

From SSDP response we have:
		ST: upnp:rootdevice
		Location: http://192.168.1.254:1990/WFADevice.xml

Create directory "192.168.1.254:1990/"

Write "SSDP-upnp-rootdevice.txt" (replace : with -)

Write "WFADevice.xml"

Enumerate services from device description
	<SCPDURL>/x_wfawlanconfig.xml</SCPDURL>

Fetch and write each file with path.

 */
void initUPnP()
{
	controlPoint.beginSearch(UPnP::RootDeviceUrn(), onSsdp);
	timer.initializeMs<20000>(InterruptCallback([]() {
		controlPoint.cancelSearch();
		String urn = urnFetchList.at(0);
		urnFetchList.remove(0);
		urnDoneList.add(urn);
		if(urnFetchList.count() > 0) {
			urn = urnFetchList.at(0);
			controlPoint.beginSearch(UPnP::Urn(urn), onSsdp);
		}
	}));
	//	timer.start();
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
	assert(kind == urn.kind);
}

void testUrn()
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

void parseXml()
{
	XML::Document doc;
	XML::deserialize(doc, gatedesc_xml);
	auto node = XML::getNode(doc, "/device/deviceList");
	debug_e("Node %s found", node ? "" : "NOT");
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
