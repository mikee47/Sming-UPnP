#include <Wemo.h>
#include <Platform/Station.h>

namespace UPnP
{
namespace Belkin
{
void BasicEventService::handleAction(Envelope& env)
{
	auto act = env.actionName();
	if(act == "GetBinaryState") {
		env.createResponse(act);
		env.addArg("BinaryState", controllee().getState());
		return;
	}

	if(act == "SetBinaryState") {
		bool state;
		if(env.getArg("BinaryState", state)) {
			controllee().setState(state);
			env.createResponse(act);
		}
		return;
	}
}

void MetaInfoService::handleAction(Envelope& env)
{
	// TODO
	//	auto res = createEnvelope(action);
	//	XML::appendNode(res, "BinaryState", 1);
	//	return res;
}

String Controllee::getField(Field desc) const
{
	switch(desc) {
	case Field::friendlyName:
		return name_;

	case Field::serialNumber: {
		String s = F("221517K01017xxxx");
		s[12] = hexchar((id_ >> 16) & 0x0f);
		s[13] = hexchar((id_ >> 8) & 0x0f);
		s[14] = hexchar((id_ >> 4) & 0x0f);
		s[15] = hexchar((id_ >> 0) & 0x0f);
		return s;
	}
	case Field::UDN: {
		String s;
		s += "uuid:Socket-1_0-";
		s += getField(Field::serialNumber);
		return s;
	}
	case Field::baseURL: {
		String url;
		url += F("/wemo/");
		url += id_;
		url += '/';
		return url;
	}
	case Field::productNameAndVersion:
		return F("Wemo/1.0");

	default:
		return Device::getField(desc);
	}
}

bool Controllee::formatMessage(Message& msg, MessageSpec& ms)
{
	msg["01-NLS"] = F("b9200ebb-736d-4b93-bf03-835149d13983");
	msg["OPT"] = F("\"http://schemas.upnp.org/upnp/1/0/\"; ns=01");
	msg["X-User-Agent"] = F("redsonic");
	return Device::formatMessage(msg, ms);
}

} // namespace Belkin
} // namespace UPnP
