#pragma once

#include <Network/UPnP/Belkin/ClassGroup.h>
#include <Timer.h>

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
			// You could also put this in the schema
			return F("urn:Belkin:serviceId:basicevent1");
		default:
			return basicevent1Template::getField(desc);
		}
	}

	Controllee& controllee()
	{
		return reinterpret_cast<Controllee&>(device());
	}

	Error getBinaryState(GetBinaryState::Response response)
	{
		response.setBinaryState(controllee().getState());
		return Error::Success;
	}

	Error setBinaryState(bool state, SetBinaryState::Response response)
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

	Error getMetaInfo(GetMetaInfo::Response response)
	{
		/*
		 * Not implemented here, but as a test defer the response for
		 * a short time.
		 * The standard HttpServer connection timeout is 2 seconds, so
		 * ID < 1 should succeed, whereas 2+ will probably fail as connection
		 * will have been destroyed by the time we call complete().
		 */
		auto timeout = controllee().id() * 1800;
		++responseNumber;

		auto timer = new AutoDeleteTimer;
		timer->initializeMs(timeout, [this, response]() {
			debug_e("Completing request");
			String s;
			s += F("Our meta info response from ");
			s += controllee().friendlyName();
			s += F(". This is request #");
			s += responseNumber;
			s += F(". Failure count = ");
			s += failureCount;
			s += '.';
			response.setMetaInfo(s);
			if(!response.complete(Error::Success)) {
				++failureCount;
			}
		});
		timer->startOnce();
		debug_e("Pending request...");
		return Error::Pending;
	}

private:
	unsigned responseNumber{0};
	static unsigned failureCount;
};

} // namespace Belkin
} // namespace UPnP
