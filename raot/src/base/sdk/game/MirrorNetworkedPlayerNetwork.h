#pragma once
#include <sdk.h>
class MirrorNetworkedPlayerNetwork : public II::MonoBehaviour
{
public:
	static auto init() -> void;
	inline static IF* field_sync_lookat = nullptr;
	inline  II::Vector3* get_sync_lookat() {

		return reinterpret_cast<II::Vector3*>(reinterpret_cast<std::uint8_t*>(this) + field_sync_lookat->offset);
	}
};

