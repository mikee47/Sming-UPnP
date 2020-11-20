#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <Data/BitSet.h>
#include <Data/CString.h>
#include "Fetch.h"

#ifdef ARCH_HOST
#ifdef __WIN32__
#include <io.h>
#endif

#include <hostlib/CommandLine.h>
#include <sys/stat.h>
#include <Data/Stream/HostFileStream.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
UPnP::ControlPoint controlPoint(0x20000);
Timer queueTimer;
Timer statusTimer;
Timer fetchTimer;

void beginNextSearch();
void fetchNextDescription();

enum class Option {
	networkScan,
	writeDeviceTree,
	writeDeviceSchema,
	writeServiceSchema,
	overwriteExisting,
};

BitSet<uint32_t, Option> options;

constexpr unsigned maxDescriptionFetchAttempts{3};
constexpr unsigned descriptionFetchTimeout{5000};

FetchList descriptionQueue("Descriptions");
FetchList ssdpQueue("SSDP messages");

// These can be overridden via environment variables
#define ENV_DEVICE_DIR "DEVICE_DIR"
#define ENV_SCHEMA_DIR "SCHEMA_DIR"

const char* deviceDir = "out/upnp/devices";
const char* schemaDir = "out/upnp/schema";

//IMPORT_FSTR(gatedesc_xml, PROJECT_DIR "/devices1/192.168.1.254/1900/gatedesc.xml")

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

#ifdef ARCH_HOST
bool exist(const String& path)
{
	struct stat s;
	return ::stat(path.c_str(), &s) >= 0;
}

void makedirs(const String& path)
{
	String buf = path;
	buf.replace('\\', '/');
	char* s = buf.begin();
	char* p = s;
	while((p = strchr(p, '/')) != nullptr) {
		*p = '\0';
#ifdef __linux__
		mkdir(s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
		mkdir(s);
#endif
		*p++ = '/';
	}
}
#endif

Print* openStream(String path)
{
#ifdef ARCH_HOST
	path.replace(':', '-');
	path.replace('?', '-');
	path.replace('&', '-');
	path.replace(' ', '_');

	if(!options[Option::overwriteExisting] && exist(path)) {
		Serial.print(F("Skipping existing file '"));
		Serial.print(path);
		Serial.println("'");
		return nullptr;
	}

	makedirs(path);
	Serial.print(_F("Writing "));
	Serial.println(path);
	auto fs = new HostFileStream;
	if(fs->open(path, eFO_CreateNewAlways | eFO_WriteOnly)) {
		return fs;
	}
	debug_e("Failed to create '%s'", path.c_str());
	delete fs;
#endif

	return nullptr;
}

void writeServiceSchema(XML::Document& scpd, const String& serviceType)
{
	XML::appendNode(scpd.first_node(), "serviceType", serviceType);

	UPnP::Urn urn(serviceType);
	String path = schemaDir;
	path += "/service/";
	path += urn.domain;
	path += '/';
	path += urn.type;
	path += urn.version;
	path += ".xml";

	auto fs = openStream(path);
	if(fs != nullptr) {
		XML::serialize(scpd, *fs, true);
		delete fs;
	}
}

void writeDeviceSchema(XML::Node* device, const String& deviceType)
{
	auto node = device->first_node("manufacturer");
	assert(node != nullptr);
	String manufacturer(node->value(), node->value_size());

	node = device->first_node("friendlyName");
	assert(node != nullptr);
	String friendlyName(node->value(), node->value_size());

	auto doc = device->document();
	auto root = device->document()->first_node();
	assert(root != nullptr);
	auto attr = root->first_attribute();
	while(attr != nullptr) {
		auto clone = doc->allocate_attribute(attr->name(), attr->value(), attr->name_size(), attr->value_size());
		device->append_attribute(clone);
		attr = attr->next_attribute();
	}

	UPnP::Urn urn(deviceType);
	String path = schemaDir;
	path += "/device/";
	path += manufacturer;
	path += '/';
	path += friendlyName;
	path += '/';
	path += urn.type;
	path += urn.version;
	path += ".xml";
	auto fs = openStream(path);
	if(fs != nullptr) {
		XML::serialize(*device, *fs, true);
		delete fs;
	}
}

void parseDevice(XML::Node* device, const Fetch& f)
{
	auto node = device->first_node("deviceType");
	assert(node != nullptr);
	String deviceType(node->value(), node->value_size());
	if(deviceType == nullptr) {
		Serial.println("*** deviceType NOT found ***");
		return;
	}
	ssdpQueue.add(UPnP::Urn{deviceType});

	if(options[Option::writeDeviceSchema]) {
		writeDeviceSchema(device, deviceType);
	}

	auto serviceList = device->first_node("serviceList");
	debug_i("serviceList %sfound", serviceList ? "" : "NOT ");
	if(serviceList != nullptr) {
		auto svc = serviceList->first_node();
		while(svc != nullptr) {
			node = svc->first_node("serviceType");
			assert(node != nullptr);
			String svcType(node->value(), node->value_size());
			ssdpQueue.add(UPnP::Urn{svcType});

			auto node = svc->first_node("SCPDURL");
			if(node == nullptr) {
				Serial.println("*** SCPDURL missing ***");
			} else {
				Url url(f.url);
				url.Path = String(node->value(), node->value_size());
				descriptionQueue.add({UPnP::Urn::Kind::service, String(url), String(f.root), svcType});
			}

			svc = svc->next_sibling();
		}
	}

	auto deviceList = device->first_node("deviceList");
	if(deviceList != nullptr) {
		auto dev = deviceList->first_node();
		while(dev != nullptr) {
			parseDevice(dev, f);
			dev = dev->next_sibling();
		}
	}
}

void parseDescription(XML::Document& description, const Fetch& f)
{
	if(f.kind == UPnP::Urn::Kind::service) {
		if(options[Option::writeServiceSchema]) {
			writeServiceSchema(description, f.path);
		}
	} else {
		auto device = XML::getNode(description, "/device");
		if(device == nullptr) {
			Serial.println(F("device  NOT found"));
		} else {
			parseDevice(device, f);
		}
	}
}

void scheduleFetch(unsigned delay = 2000)
{
	queueTimer.initializeMs(delay, fetchNextDescription).startOnce();
}

void fetchNextDescription()
{
	Fetch* fetch{nullptr};

	for(;;) {
		Fetch& f = descriptionQueue.find(Fetch::State::pending);
		if(!f) {
			beginNextSearch();
			return;
		}

		if(f.attempts < maxDescriptionFetchAttempts) {
			++f.attempts;
			Serial.print(_F("Fetching '"));
			Serial.println(f.toString());
			fetch = &f;
			break;
		}
		debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
		f.finished(false);
	}

	auto callback = [fetch](HttpConnection& connection, XML::Document* description) {
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

		auto& f = *fetch;

		if(!connection.getResponse()->isSuccess()) {
			// Fetch failed, move to end of queue
			if(f.attempts >= maxDescriptionFetchAttempts) {
				debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
				f.finished(false);
			} else {
				Serial.print(_F("Fetch '"));
				Serial.print(f.url);
				Serial.println("' failed, re-trying");
			}
		} else if(description != nullptr) {
			if(options[Option::writeDeviceTree]) {
				// Write description
				auto fs = openStream(f.fullPath());
				if(fs != nullptr) {
					XML::serialize(*description, *fs, true);
					delete fs;
				}
			}

			f.finished(true);

			parseDescription(*description, f);
		}

		scheduleFetch();
	};

	assert(fetch != nullptr);
	if(controlPoint.requestDescription(fetch->url, callback)) {
		fetchTimer.initializeMs<descriptionFetchTimeout>(fetchNextDescription);
		fetchTimer.startOnce();
	} else {
		scheduleFetch();
	}
}

void onSsdp(SSDP::BasicMessage& msg)
{
	String location = msg[HTTP_HEADER_LOCATION];

	Fetch f;
	f.kind = UPnP::Urn::Kind::device;
	f.url = location;

	if(ssdpQueue.contains(f)) {
		debug_d("%s - ignoring, already processed", location.c_str());
		return;
	}

	// Create root directory for device
	String root = deviceDir;
	root += '/';
	Url url(location);
	root += url.Host;
	root += '/';
	root += url.Port;
	root += '/';

	f.root = root;
	f.path = url.getRelativePath();

	auto deviceType = msg["ST"];
	if(deviceType == nullptr) {
		deviceType = msg["NT"];
	}

	UPnP::Urn urn(deviceType);
	f.kind = UPnP::Urn::Kind::device;

	if(options[Option::writeDeviceTree]) {
		// Write SSDP response
		String filename;
		filename += "ssdp-";
		filename += deviceType;
		filename += ".txt";

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
	}

	f.finished(true);

	descriptionQueue.add(f);
	scheduleFetch();
}

void printQueue(const FetchList& list)
{
	Serial.println(list.toString());
	for(unsigned i = 0; i < list.count(); ++i) {
		Serial.print("  ");
		Serial.println(list[i].toString());
	}
}

void printScanSummary()
{
	printQueue(ssdpQueue);
	printQueue(descriptionQueue);
}

void beginNextSearch()
{
	controlPoint.cancelSearch();
	auto& f = ssdpQueue.find(Fetch::State::pending);
	if(!bool(f)) {
		Serial.println(_F("ALL DONE"));
		printScanSummary();
		System.restart(2000);
		return;
	}

	controlPoint.beginSearch(f.urn(), onSsdp);
	f.finished(true);
}

void scan(const UPnP::Urn& urn)
{
	options = Option::networkScan | Option::writeDeviceTree | Option::writeDeviceSchema | Option::writeServiceSchema;
	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP([urn](IpAddress ip, IpAddress netmask, IpAddress gateway) {
		debugf("GotIP: %s", ip.toString().c_str());

		Serial.print("Scanning from ");
		Serial.println(urn.toString());

		ssdpQueue.add(urn);
		beginNextSearch();

		statusTimer.initializeMs<10000>(InterruptCallback([]() {
			Serial.println();
			Serial.println();
			Serial.println(_F("** Queue status **"));
			Serial.println(descriptionQueue.toString());
			Serial.println(ssdpQueue.toString());
			Serial.println(_F("** ------------ **"));
			Serial.println();
			Serial.println();
		}));
		statusTimer.start();
	});
}

#ifdef ARCH_HOST

void parseFile(const String& filename, const Fetch& f)
{
	XML::Document doc;
	HostFileStream fs(f.root + filename);
	String content = fs.readString(fs.available());
	XML::deserialize(doc, content);
	parseDescription(doc, f);
}

void parseXml(String root, String filename)
{
	options = Option::writeDeviceSchema | Option::writeServiceSchema;

	if(root.length() == 0 || filename.length() == 0) {
		return;
	}
	root.replace('\\', '/');
	filename.replace('\\', '/');
	auto len = root.length();
	if(root[len - 1] == '/') {
		root.setLength(len - 1);
	}
	if(filename[0] != '/') {
		filename = "/" + filename;
	}

	Fetch f(UPnP::Urn::Kind::device, "file://.", root, nullptr);
	parseFile(filename, f);
	while((f = descriptionQueue.find(Fetch::State::pending))) {
		Url url(f.url);
		parseFile(url.Path, f);
		f.finished(true);
	}
}

void help()
{
	m_printf(_F("\n"
				"UPnP Scan Utility. Options:\n"
				"  scan   urn                Perform a local network scan (default is upnp:rootdevice) \n"
				"  parse  root filenames...  Parse XML files from given root directory\n"
				"\n"));
}

/*
 * Return true to continue execution, false to quit.
 */
bool parseCommands()
{
	auto parameters = commandLine.getParameters();
	if(parameters.count() == 0) {
		help();
		return false;
	}

	String cmd = parameters[0].text;
	if(cmd == "scan") {
		auto urn = UPnP::RootDeviceUrn();
		if(parameters.count() > 1) {
			auto str = parameters[1].text;
			if(!urn.decompose(str)) {
				m_printf("Invalid URN: %s\n", str);
				return false;
			}
		}
		scan(urn);
		return true;
	}

	if(cmd == "parse") {
		if(parameters.count() < 3) {
			Serial.println(F("** Missing parameters"));
			help();
		} else {
			String root = parameters[1].text;
			for(unsigned i = 2; i < parameters.count(); ++i) {
				parseXml(root, parameters[i].text);
			}
		}
		return false;
	}

	help();
	return false;
}

#endif

} // namespace

void init()
{
	Serial.setTxBufferSize(4096);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

#ifdef ARCH_HOST

	auto p = getenv(ENV_DEVICE_DIR);
	if(p != nullptr && *p != '\0') {
		deviceDir = p;
	}
	m_printf("DEVICE_DIR = '%s'\n", deviceDir);

	p = getenv(ENV_SCHEMA_DIR);
	if(p != nullptr && *p != '\0') {
		schemaDir = p;
	}
	m_printf("SCHEMA_DIR = '%s'\n", schemaDir);

	if(!parseCommands()) {
		System.restart();
	}

#else
	scan();
#endif
}
