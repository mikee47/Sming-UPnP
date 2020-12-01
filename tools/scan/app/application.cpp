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
UPnP::ControlPoint controlPoint(
#ifdef ARCH_HOST
	0x20000
#else
	8192
#endif
);
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
constexpr unsigned descriptionFetchTimeout{2000};
constexpr unsigned fetchRetryDelay{1000};
constexpr unsigned fetchInterval{250};

FetchList descriptionQueue("Descriptions");
FetchList ssdpQueue("SSDP messages");

// These can be overridden via environment variables
#define ENV_DEVICE_DIR "DEVICE_DIR"
#define ENV_SCHEMA_DIR "SCHEMA_DIR"

#define OUTPUT_DIR "out/upnp"
const char* logFileName = OUTPUT_DIR "/log.txt";
const char* deviceDir = OUTPUT_DIR "/devices";
const char* schemaDir = OUTPUT_DIR "/schema";

template <typename T> void print(const T& arg)
{
	String s(arg);
	m_nputs(s.c_str(), s.length());
}

void println()
{
	m_puts("\r\n");
}

template <typename T> void println(const T& arg)
{
	print(arg);
	println();
}

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

String& validate(String& path)
{
	path.replace(':', '-');
	path.replace('?', '-');
	path.replace('&', '-');
	path.replace(' ', '_');
	return path;
}
#endif

Print* openStream(String path)
{
#ifdef ARCH_HOST
	validate(path);
	makedirs(path);
	debug_i("Writing %s", path.c_str());
	auto fs = new HostFileStream;
	if(fs->open(path, eFO_CreateNewAlways | eFO_WriteOnly)) {
		return fs;
	}
	debug_e("Failed to create '%s'", path.c_str());
	delete fs;
#endif

	return nullptr;
}

void writeSchema(XML::Node* object, const Urn& objectType)
{
	String path = schemaDir;
	path += '/';
	path += objectType.domain;
	path += '/';
	path += toString(objectType.kind);
	path += '/';
	path += objectType.type;
	path += objectType.version;
	path += ".xml";

	auto fs = openStream(path);
	if(fs != nullptr) {
		XML::serialize(*object, *fs, true);
		delete fs;
	}
}

void writeServiceSchema(XML::Document& scpd, const String& serviceType, const String& serviceId)
{
	XML::appendNode(scpd.first_node(), "serviceType", serviceType);
	XML::appendNode(scpd.first_node(), "serviceId", serviceId);
	writeSchema(&scpd, Urn(serviceType));
}

void writeDeviceSchema(XML::Node* device, const String& deviceType)
{
	auto doc = device->document();
	auto root = device->document()->first_node();
	assert(root != nullptr);
	auto attr = root->first_attribute();
	while(attr != nullptr) {
		auto clone = doc->allocate_attribute(attr->name(), attr->value(), attr->name_size(), attr->value_size());
		device->append_attribute(clone);
		attr = attr->next_attribute();
	}

	writeSchema(device, Urn(deviceType));
}

void checkExisting(Fetch& desc)
{
#ifdef ARCH_HOST
	if(desc.state == Fetch::State::success || options[Option::overwriteExisting]) {
		return;
	}

	String path = desc.fullPath();
	validate(path);
	if(!exist(path)) {
		return;
	}

	debug_w("Skipping '%s': already fetched", path.c_str());
	desc.state = Fetch::State::skipped;
#endif
}

void parseDevice(XML::Node* device, const Fetch& f)
{
	String deviceType = XML::getValue(device, "deviceType");
	if(!deviceType) {
		debug_e("*** deviceType NOT found ***");
		return;
	}
	ssdpQueue.add(Urn{deviceType});

	if(options[Option::writeDeviceSchema]) {
		writeDeviceSchema(device, deviceType);
	}

	auto serviceList = device->first_node("serviceList");
	debug_i("serviceList %sfound", serviceList ? "" : "NOT ");
	if(serviceList != nullptr) {
		auto svc = serviceList->first_node();
		while(svc != nullptr) {
			String svcType = XML::getValue(svc, "serviceType");
			if(!svcType) {
				debug_e("*** serviceType missing ***");
			} else {
				ssdpQueue.add(Urn{svcType});

				Url url(f.url);
				url.Path = XML::getValue(svc, "SCPDURL");
				if(!url.Path) {
					debug_e("*** SCPDURL missing ***");
				} else {
					auto& desc = descriptionQueue.add({Urn::Kind::service, String(url), String(f.root), svcType});
					desc.id = XML::getValue(svc, "serviceId");
					checkExisting(desc);
				}
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
	if(f.kind == Urn::Kind::service) {
		if(options[Option::writeServiceSchema]) {
			writeServiceSchema(description, f.path, f.id);
		}
	} else {
		auto device = XML::getNode(description, "/device");
		if(device == nullptr) {
			debug_e("device  NOT found");
		} else {
			parseDevice(device, f);
		}
	}
}

void scheduleFetch(unsigned delay)
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
			debug_i("Fetching '%s', attempt #%u", f.toString().c_str(), f.attempts);
			fetch = &f;
			break;
		}
		debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
		f.state = Fetch::State::failed;
	}

	auto callback = [fetch](HttpConnection& connection, XML::Document* description) {
#if DEBUG_VERBOSE_LEVEL == DBG
		println();
		println();
		println(F("====== BEGIN ======"));
		print(F("Remote IP: "));
		print(connection.getRemoteIp().toString());
		print(':');
		println(connection.getRemotePort());

		println(F("Request: "));
		println(connection.getRequest()->toString());
		println();

		println(connection.getResponse()->toString());
		println();

		println(F("Content: "));
		String content;
		if(description == nullptr) {
			content = connection.getResponse()->getBody();
		} else {
			content = XML::serialize(*description, true);
		}
		println(content);
		println(F("======  END  ======"));
#endif

		auto& f = *fetch;

		unsigned fetchDelay{fetchInterval};

		auto response = connection.getResponse();
		if(!response->isSuccess()) {
			// Fetch failed, move to end of queue
			if(f.attempts >= maxDescriptionFetchAttempts || response->code == HTTP_STATUS_NOT_FOUND) {
				debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
				f.state = Fetch::State::failed;
			} else {
				debug_w("Fetch '%s' failed, re-trying", f.url.c_str());
				fetchDelay = fetchRetryDelay;
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

			f.state = Fetch::State::success;

			parseDescription(*description, f);
		}

		scheduleFetch(fetchDelay);
	};

	assert(fetch != nullptr);

	unsigned fetchDelay;
	if(controlPoint.requestDescription(fetch->url, callback)) {
		fetchDelay = descriptionFetchTimeout;
	} else {
		fetchDelay = fetchRetryDelay;
	}

	scheduleFetch(fetchDelay);
}

Fetch createDescFetch(const String& location)
{
	Fetch f;
	f.kind = Urn::Kind::device;
	f.url = location;

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
	return f;
}

void onSsdp(SSDP::BasicMessage& msg)
{
	String location = msg[HTTP_HEADER_LOCATION];

	auto f = createDescFetch(location);

	auto deviceType = msg["ST"];
	if(deviceType == nullptr) {
		deviceType = msg["NT"];
	}

	Urn urn(deviceType);

	if(options[Option::writeDeviceTree]) {
		// Write SSDP response
		String filename;
		filename += "ssdp-";
		filename += deviceType;
		filename += ".txt";

		auto fs = openStream(f.root + filename);
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

	auto& desc = descriptionQueue.add(f);
	checkExisting(desc);

	scheduleFetch(fetchInterval);
}

void printQueue(const FetchList& list)
{
	println(list.toString());
	for(unsigned i = 0; i < list.count(); ++i) {
		print("  ");
		println(list[i].toString());
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
		println(F("ALL DONE"));
		printScanSummary();
		System.restart(2000);
		return;
	}

	controlPoint.beginSearch(f.urn(), onSsdp);
	f.state = Fetch::State::success;
}

void scan(const Urn& urn)
{
	options = Option::networkScan | Option::writeDeviceTree | Option::writeDeviceSchema | Option::writeServiceSchema;
	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP([urn](IpAddress ip, IpAddress netmask, IpAddress gateway) {
		debugf("GotIP: %s", ip.toString().c_str());

		print("Scanning from ");
		println(urn.toString());

		ssdpQueue.add(urn);
		beginNextSearch();

		statusTimer.initializeMs<10000>(InterruptCallback([]() {
			println();
			println();
			println(F("** Queue status **"));
			println(descriptionQueue.toString());
			println(ssdpQueue.toString());
			println(F("** ------------ **"));
			println();
			println();
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

	Fetch f(Urn::Kind::device, "file://.", root, nullptr);
	parseFile(filename, f);
	while(auto& f = descriptionQueue.find(Fetch::State::pending)) {
		Url url(f.url);
		parseFile(url.Path, f);
		f.state = Fetch::State::success;
	}
}

void help()
{
	println();
	println(F("UPnP Scan Utility. Options:"));
	println(F("  scan   urn                Perform a local network scan (default is upnp:rootdevice)"));
	println(F("  fetch  URL(s)...          Fetch descriptions"));
	println(F("  parse  root filenames...  Parse XML files from given root directory"));
	println();
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
		auto urn = RootDeviceUrn();
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

	if(cmd == "fetch") {
		if(parameters.count() < 1) {
			m_printf("No URL(s) specified\r\n");
			return false;
		}

		for(unsigned i = 1; i < parameters.count(); ++i) {
			descriptionQueue.add(createDescFetch(parameters[i].text));
		}

		scheduleFetch(fetchInterval);
		return true;
	}

	if(cmd == "parse") {
		if(parameters.count() < 3) {
			println(F("** Missing parameters"));
			help();
		} else {
			String root = parameters[1].text;
			for(unsigned i = 2; i < parameters.count(); ++i) {
				parseXml(root, parameters[i].text);
			}
			printQueue(descriptionQueue);
		}
		return false;
	}

	help();
	return false;
}

nputs_callback_t previous_nputs_callback;
HostFileStream log;

void openLogFile()
{
	makedirs(logFileName);
	log.open(logFileName, eFO_CreateNewAlways | eFO_Append | eFO_WriteOnly);
	log.println();
	log.println();
	log.print(F("Log opened: "));
	log.println(SystemClock.getSystemTimeString());

	previous_nputs_callback = m_setPuts([](const char* str, size_t length) {
		log.write(reinterpret_cast<const uint8_t*>(str), length);
		return previous_nputs_callback(str, length);
	});
}

#endif // ARCH_HOST

} // namespace

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

#ifdef ARCH_HOST
	openLogFile();

	auto p = getenv(ENV_DEVICE_DIR);
	if(p != nullptr && *p != '\0') {
		deviceDir = p;
	}
	m_printf("DEVICE_DIR = '%s'\r\n", deviceDir);

	p = getenv(ENV_SCHEMA_DIR);
	if(p != nullptr && *p != '\0') {
		schemaDir = p;
	}
	m_printf("SCHEMA_DIR = '%s'\r\n", schemaDir);

	if(!parseCommands()) {
		System.restart();
	}

#else
	scan(RootDeviceUrn());
#endif
}
