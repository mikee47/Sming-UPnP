#pragma once

#include <Network/UPnP/Belkin/ClassGroup.h>
#include <Data/Stream/FlashMemoryStream.h>

namespace UPnP
{
namespace Belkin
{
class Controllee : public device::controllee1Template<Controllee>
{
public:
	using StateChange = Delegate<void(Controllee& device)>;

	Controllee(unsigned id, const String& name);

	void onStateChange(StateChange delegate)
	{
		stateChange = delegate;
	}

	unsigned id() const
	{
		return id_;
	}

	virtual bool getState() const
	{
		return state_;
	}

	virtual void setState(bool state)
	{
		state_ = state;
		if(stateChange) {
			stateChange(*this);
		}
	}

	String getField(Field desc) const override;

	String getUrlBasePath() const override
	{
		String path = Device::getUrlBasePath();
		path += '/';
		path += id_;
		return path;
	}

	bool formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms) override;

private:
	unsigned id_{0};
	String name_;
	bool state_{false};
	StateChange stateChange;
};

class BasicEventService : public service::basicevent1Template<BasicEventService>
{
public:
	using basicevent1Template::basicevent1Template;

	String getField(Field desc) const override
	{
		switch(desc) {
		case Field::serviceId:
			return F("urn:Belkin:serviceId:basicevent1");
		default:
			return basicevent1Template::getField(desc);
		}
	}

	Controllee& controllee()
	{
		return reinterpret_cast<Controllee&>(device());
	}

	Error getBinaryState(GetBinaryState::Result result)
	{
		result.setBinaryState(controllee().getState());
		return Error::Success;
	}

	Error setBinaryState(bool state, SetBinaryState::Result result)
	{
		controllee().setState(state);
		return Error::Success;
	}
};

class MetaInfoService : public service::metainfo1Template<MetaInfoService>
{
public:
	using metainfo1Template::metainfo1Template;

	String getField(Field desc) const override
	{
		switch(desc) {
		case Field::serviceId:
			return F("urn:Belkin:serviceId:metainfo1");
		default:
			return Service::getField(desc);
		}
	}

	Controllee& controllee()
	{
		return reinterpret_cast<Controllee&>(device());
	}

	Error getMetaInfo(GetMetaInfo::Result result)
	{
		return Error::ActionNotImplemented;
	}
};

} // namespace Belkin
} // namespace UPnP
