#pragma once
#include "sdk.h"
class AoTNetworkModule : public II::MonoBehaviour
{
public:
	template <typename T>
	static inline T* get_instance(I::Class* pClass) {
		T* result = nullptr;
		II::Dictionary<II::CsType*, AoTNetworkModule*>* instances = nullptr;
		auto type = pClass->GetType();
		field_Instances->GetStaticValue(&instances);
		if (instances != nullptr && instances->iCount != 0 && instances->pEntries != nullptr)
		{
			for (size_t i = 0; i < instances->iCount; i++)
			{
				auto key = instances->GetKeyByIndex(i);
				if (!key)
				{
					//std::cout << "index " << i << " :" << key->FormatTypeName()->ToString() << std::endl;
					continue;
				}
				if (key == type)
				{
					//std::cout << "found !" << std::endl;
					//result = reinterpret_cast<T*>(instances->GetValueByIndex(i));
					return reinterpret_cast<T*>(instances->GetValueByIndex(i));
				}
			}
		}
		return result;
	}

	static auto init() -> void;
	static inline I::Class* cached;
	static inline I::Field* field_Instances;
};
