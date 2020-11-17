#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>

#ifdef ARCH_HOST
#include <io.h>
#include <Data/Stream/HostFileStream.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
UPnP::ControlPoint controlPoint(8192);
Timer queueTimer;
Timer statusTimer;

constexpr unsigned maxDescriptionFetchAttempts{3};

// Fetch one description file at a time to avoid swamping the TCP stack
struct Fetch {
	String url;
	String root;
	String path;
	unsigned attempts{0};

	bool operator==(const Fetch& other)
	{
		return path == other.path;
	}
};

Vector<Fetch> descriptionFetchList;
Vector<Fetch> descriptionDoneList;
Vector<String> ssdpFetchList;
Vector<String> ssdpDoneList;

void queueSsdp(const String& urn)
{
	if(!ssdpDoneList.contains(urn) && !ssdpFetchList.contains(urn)) {
		Serial.print(_F("Queuing SSDP "));
		Serial.println(urn);
		ssdpFetchList.add(urn);
	}
}

static const char* baseOutputDir = "devices";

//IMPORT_FSTR(gatedesc_xml, PROJECT_DIR "/devices1/192.168.1.254/1900/gatedesc.xml")

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void makedirs(const String& path)
{
#ifdef ARCH_HOST
	String buf = path;
	buf.replace('\\', '/');
	char* s = buf.begin();
	char* p = s;
	while((p = strchr(p, '/')) != nullptr) {
		*p = '\0';
		mkdir(s);
		*p++ = '/';
	}
#endif
}

void checkPath(String& path)
{
	path.replace(':', '-');
	path.replace('?', '-');
	path.replace('&', '-');
}

Print* openStream(const String& path)
{
#ifdef ARCH_HOST
	makedirs(path);
	Serial.print(_F("Writing "));
	Serial.println(path);
	auto fs = new HostFileStream;
	if(fs->open(path, eFO_CreateNewAlways | eFO_WriteOnly)) {
		return fs;
	}
	debug_e("Failed to create '%s'", path.c_str());
	delete fs;
	return nullptr;
#endif
}

void beginNextSearch();
void fetchNextDescription();

void queueDescription(const String& location, const String& root)
{
	Url url(location);
	Fetch f{location, root, url.getRelativePath()};
	checkPath(f.path);
	if(descriptionFetchList.contains(f)) {
		debug_d("%s already queued", location.c_str());
		return;
	}
	if(descriptionDoneList.contains(f)) {
		debug_d("%s already fetched", location.c_str());
		return;
	}

	descriptionFetchList.add(f);

	Serial.print(_F("Queuing '"));
	Serial.print(location);
	Serial.print(" (");
	Serial.print(descriptionFetchList.count());
	Serial.println(')');

	if(descriptionFetchList.count() == 1) {
		queueTimer.initializeMs<2000>(fetchNextDescription).startOnce();
	}
}

void parseDevice(XML::Node* device, const Fetch& f)
{
	String deviceType = device->first_node("deviceType")->value();
	debug_e("deviceType %sfound", deviceType ? "" : "NOT ");
	queueSsdp(deviceType);

	auto serviceList = device->first_node("serviceList");
	debug_i("serviceList %sfound", serviceList ? "" : "NOT ");
	if(serviceList != nullptr) {
		auto svc = serviceList->first_node();
		while(svc != nullptr) {
			String svcType = svc->first_node("serviceType")->value();
			queueSsdp(svcType);

			auto node = svc->first_node("SCPDURL");
			if(node == nullptr) {
				debug_e("*** SCPDURL missing ***");
			} else {
				Url url(f.url);
				url.Path = node->value();
				queueDescription(String(url), f.root);
			}

			svc = svc->next_sibling();
		}
	}

	auto deviceList = device->first_node("deviceList");
	debug_i("deviceList %sfound", deviceList ? "" : "NOT ");
	if(deviceList != nullptr) {
		auto dev = deviceList->first_node();
		while(dev != nullptr) {
			parseDevice(dev, f);
			dev = dev->next_sibling();
		}
	}
}

void parseDescription(XML::Document& doc, const Fetch& f)
{
	auto device = XML::getNode(doc, "/device");
	debug_e("device %sfound", device ? "" : "NOT ");
	if(device != nullptr) {
		parseDevice(device, f);
	}
}

void fetchNextDescription()
{
	if(descriptionFetchList.count() == 0) {
		queueTimer.initializeMs<2000>(beginNextSearch).startOnce();
		return;
	}

	Fetch f = descriptionFetchList.at(0);

	Serial.print(_F("Fetching '"));
	Serial.print(f.url);
	Serial.print(_F("' to '"));
	Serial.print(f.root);
	Serial.print(f.path);
	Serial.println('\'');

	auto callback = [](HttpConnection& connection, XML::Document* description) {
#if DEBUG_VERBOSE_LEVEL == DBG
		Serial.println();
		Serial.println();
		Serial.println(_F("====== BEGIN ======"));
		Serial.print(_F("Remote IP: "));
		Serial.print(connection.getRemoteIp());
		Serial.print(':');
		Serial.println(connection.getRemotePort());

		Serial.println(_F("Request: "));
		Serial.println(connection.getRequest()->toString());
		Serial.println();

		Serial.println(connection.getResponse()->toString());
		Serial.println();

		if(description != nullptr) {
			Serial.println(_F("Content:"));
			XML::serialize(*description, Serial, true);
			Serial.println();
			Serial.println(_F("======  END  ======"));
		}
#endif

		Fetch f = descriptionFetchList.at(0);
		descriptionFetchList.remove(0);

		if(!connection.getResponse()->isSuccess()) {
			// Fetch failed, move to end of queue
			++f.attempts;
			if(f.attempts >= maxDescriptionFetchAttempts) {
				debug_e("Giving up on '%s' after %u attempts", f.attempts);
			} else {
				Serial.print(_F("Fetch '"));
				Serial.print(f.url);
				Serial.println("' failed, re-queueing");
				descriptionFetchList.add(f);
			}
		} else if(description != nullptr) {
			// Write description
			auto fs = openStream(f.root + f.path);
			if(fs != nullptr) {
				XML::serialize(*description, *fs, true);
				delete fs;
			}

			descriptionDoneList.add(f);

			parseDescription(*description, f);
		}

		queueTimer.initializeMs<1000>(fetchNextDescription).startOnce();
	};

	unsigned due = 2000;
	if(controlPoint.requestDescription(f.url, callback)) {
		due = 10000;
	}
	queueTimer.initializeMs(due, fetchNextDescription).startOnce();
}

void onSsdp(SSDP::BasicMessage& msg)
{
	String location = msg[HTTP_HEADER_LOCATION];

	if(ssdpDoneList.indexOf(location) >= 0) {
		debug_d("%s - ignoring, already processed", location.c_str());
		return;
	}

	// Create root directory for device
	String root = baseOutputDir;
	root += '/';
	Url url(location);
	root += url.Host;
	root += '/';
	root += url.Port;
	root += '/';
	/*
	String path = url.getRelativePath();
	int sep = path.lastIndexOf('/');
	if(sep >= 0) {
		root.concat(path.c_str(), sep);
		root += '/';
	}
*/
	makedirs(root);

	// Write SSDP response
	auto devtype = msg["ST"];
	if(devtype == nullptr) {
		devtype = msg["NT"];
	}
	String filename;
	filename += "ssdp-";
	filename += devtype;
	filename += ".txt";
	checkPath(filename);

	auto fs = openStream(root + filename);
	if(fs != nullptr) {
		fs->print(F("Message type: "));
		fs->println(toString(msg.type));
		fs->println();
		for(unsigned i = 0; i < msg.count(); ++i) {
			fs->print(msg[i]);
		}
		delete fs;
	}

	ssdpDoneList.add(location);

	queueDescription(location, root);
}

void beginNextSearch()
{
	controlPoint.cancelSearch();
	if(ssdpFetchList.count() == 0) {
		Serial.println(_F("ALL DONE"));
		System.restart(2000);
	}

	String urn = ssdpFetchList.at(0);
	ssdpFetchList.remove(0);

	controlPoint.beginSearch(UPnP::Urn(urn), onSsdp);
	ssdpDoneList.add(urn);
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
	ssdpFetchList.add(String(UPnP::RootDeviceUrn()));
	beginNextSearch();

	statusTimer.initializeMs<10000>(InterruptCallback([]() {
		Serial.println();
		Serial.println();
		Serial.println(_F("** Queue status **"));
		Serial.print(_F("  Descriptions: fetched "));
		Serial.print(descriptionDoneList.count());
		Serial.print(_F(" of "));
		Serial.println(descriptionDoneList.count() + descriptionFetchList.count());
		Serial.print(_F("  SSDP:         fetched "));
		Serial.print(ssdpDoneList.count());
		Serial.print(_F(" of "));
		Serial.println(ssdpDoneList.count() + ssdpFetchList.count());
		Serial.println(_F("** ------------ **"));
		Serial.println();
		Serial.println();
	}));
	statusTimer.start();
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

/*
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
	parseDescription(doc, Fetch{"LOC", "ROOT", "PATH"});
}
*/

} // namespace

void init()
{
	Serial.setTxBufferSize(4096);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);

	//	parseXml();

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
