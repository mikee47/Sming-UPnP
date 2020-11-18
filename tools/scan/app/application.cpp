#include <SmingCore.h>
#include <Network/UPnP/ControlPoint.h>
#include <Data/BitSet.h>
#include <Data/CString.h>

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
UPnP::ControlPoint controlPoint(65536);
Timer queueTimer;
Timer statusTimer;
Timer fetchTimer;

void beginNextSearch();
void fetchNextDescription();

enum class Option {
	networkTree,
};

BitSet<uint32_t, Option> options = Option::networkTree;

constexpr unsigned maxDescriptionFetchAttempts{3};
constexpr unsigned descriptionFetchTimeout{5000};

// Fetch one description file at a time to avoid swamping the TCP stack
struct Fetch {
	UPnP::Urn::Kind kind{};
	String url;
	String root;
	String path;
	unsigned attempts{0};

	Fetch() = default;
	Fetch(const Fetch&) = default;

	Fetch(const String& url, const String& root, const String& path)
		: kind(UPnP::Urn(path).kind), url(url), root(root), path(path)
	{
	}

	Fetch(const UPnP::Urn& urn) : kind(urn.kind), path(urn.toString())
	{
	}

	explicit operator bool() const
	{
		return kind != UPnP::Urn::Kind::none;
	}

	bool operator==(const Fetch& other)
	{
		return path == other.path;
	}

	UPnP::Urn urn() const
	{
		return UPnP::Urn(path);
	}

	String fullPath() const
	{
		String s;
		s += root;
		s += path;
		return s;
	}

	String toString() const
	{
		String s;
		if(url) {
			s += url;
			s += F("' -> '");
			s += root;
			s += path;
			s += '\'';
		} else {
			s = path;
		}
		return s;
	}
};

class FetchList
{
public:
	FetchList(const String& name) : name(name)
	{
	}

	bool contains(const Fetch& f)
	{
		return queue.contains(f) || done.contains(f);
	}

	bool isDone(const Fetch& f)
	{
		return done.contains(f);
	}

	bool add(Fetch f)
	{
		if(f.kind > UPnP::Urn::Kind::service) {
			f.kind = UPnP::Urn::Kind::none;
		}

		if(contains(f)) {
			return false;
		}

		if(f.kind == UPnP::Urn::Kind::service) {
			//			unsigned i = 0;
			//			while(i < queue.count()) {
			//				if(queue[i].)
			//			}
			queue.insertElementAt(f, 0);
		} else {
			queue.add(f);
		}

		Serial.print(_F("Queuing '"));
		Serial.println(f.toString());

		return true;
	}

	Fetch& peek()
	{
		if(queue.count() == 0) {
			nil = Fetch();
			return nil;
		} else {
			return queue[0];
		}
	}

	Fetch pop()
	{
		if(queue.count() == 0) {
			return Fetch{};
		}

		Fetch f = queue.at(0);
		queue.remove(0);
		return f;
	}

	void finished(const Fetch& f)
	{
		done.add(f);
		queue.removeElement(f);
	}

	void finished(const String& url)
	{
		Fetch f;
		f.url = url;
		finished(f);
	}

	unsigned count() const
	{
		return queue.count();
	}

	String toString() const
	{
		String s;
		s += name;
		s += F(": fetched ");
		s += done.count();
		s += F(" of ");
		s += done.count() + queue.count();
		return s;
	}

private:
	using List = Vector<Fetch>;
	static Fetch nil;
	String name;
	List queue;
	List done;
};

Fetch FetchList::nil{};

FetchList descriptionQueue("Descriptions");
FetchList ssdpQueue("SSDP messages");
Fetch currentDesc;

static const char* deviceDir = "devices";
static const char* schemaDir = "schema";

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

Print* openStream(String path)
{
#ifdef ARCH_HOST
	path.replace(':', '-');
	path.replace('?', '-');
	path.replace('&', '-');
	path.replace(' ', '_');

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

void writeServiceSchema(XML::Document& scpd, const String& serviceType)
{
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
	debug_e("deviceType %sfound", deviceType ? "" : "NOT ");
	ssdpQueue.add(UPnP::Urn{deviceType});

	writeDeviceSchema(device, deviceType);

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
				debug_e("*** SCPDURL missing ***");
			} else {
				Url url(f.url);
				url.Path = String(node->value(), node->value_size());
				descriptionQueue.add(Fetch(String(url), String(f.root), svcType));
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

void parseDescription(XML::Document& description, const Fetch& f)
{
	if(f.kind == UPnP::Urn::Kind::service) {
		writeServiceSchema(description, f.path);
	} else {
		auto device = XML::getNode(description, "/device");
		debug_e("device %sfound", device ? "" : "NOT ");
		if(device != nullptr) {
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
	Fetch fetch;

	for(;;) {
		if(descriptionQueue.count() == 0) {
			beginNextSearch();
			return;
		}

		Fetch& f = descriptionQueue.peek();
		++f.attempts;
		if(f.attempts <= maxDescriptionFetchAttempts) {
			Serial.print(_F("Fetching '"));
			Serial.println(f.toString());
			fetch = f;
			break;
		}
		debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
		descriptionQueue.finished(f);
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

		auto& f = fetch;

		if(!connection.getResponse()->isSuccess()) {
			// Fetch failed, move to end of queue
			if(f.attempts >= maxDescriptionFetchAttempts) {
				debug_e("Giving up on '%s' after %u attempts", f.toString().c_str(), f.attempts);
				descriptionQueue.finished(f);
			} else {
				Serial.print(_F("Fetch '"));
				Serial.print(f.url);
				Serial.println("' failed, re-trying");
			}
		} else if(description != nullptr) {
			if(options[Option::networkTree]) {
				// Write description
				auto fs = openStream(f.fullPath());
				if(fs != nullptr) {
					XML::serialize(*description, *fs, true);
					delete fs;
				}
			}

			descriptionQueue.finished(f);

			parseDescription(*description, f);
		}

		scheduleFetch();
	};

	if(controlPoint.requestDescription(fetch.url, callback)) {
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
	f.kind = urn.kind;

	if(options[Option::networkTree]) {
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

	ssdpQueue.finished(f);

	descriptionQueue.add(f);
	scheduleFetch();
}

void beginNextSearch()
{
	controlPoint.cancelSearch();
	Fetch f = ssdpQueue.pop();
	if(!bool(f)) {
		Serial.println(_F("ALL DONE"));
		System.restart(2000);
	}

	controlPoint.beginSearch(f.urn(), onSsdp);
	ssdpQueue.finished(f);
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
	ssdpQueue.add(UPnP::RootDeviceUrn());
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
