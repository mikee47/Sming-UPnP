#include <Network/UPnP/schemas-sming-org/ClassGroup.h>

namespace UPnP
{
namespace schemas_sming_org
{
class TeaPot : public device::TeaPot1Template<TeaPot>
{
public:
	TeaPot(uint8_t id) : id_(id)
	{
	}

	String getField(Field desc) const override
	{
		switch(desc) {
		case Field::UDN:
			// This is the unique id of the device
			return F("uuid:68317e07-d356-455a-813b-d23f2556354a");
		case Field::serialNumber:
			return F("12345678");
		default:
			return Device::getField(desc);
		}
	}

	// Ensure URL is unique if there are multiple devices
	String getUrlBasePath() const override
	{
		String path = Device::getUrlBasePath();
		path += '/';
		path += id_;
		return path;
	}

private:
	uint8_t id_;
};

} // namespace schemas_sming_org
} // namespace UPnP
