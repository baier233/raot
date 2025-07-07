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
				/*
index 0 :AoTNetworking.Modules.MirrorClientModule
index 1 :AoTNetworking.Modules.MirrorPlayerModule
index 2 :AoTNetworking.Modules.MirrorRoundClockModule
index 3 :AoTNetworking.Modules.MirrorScoreboardModule
index 4 :AoTNetworking.Modules.MirrorGameModeCoreModule
index 5 :AoTNetworking.Modules.Chat.MirrorChatModule
index 6 :AoTNetworking.Modules.MirrorTeamCharacterModule
index 7 :AoTNetworking.Modules.MirrorPvpModule
index 8 :AoTNetworking.Modules.MirrorObjectiveModule
index 9 :AoTNetworking.Modules.MirrorProjectileModule
index 10 :AoTNetworking.Modules.MirrorFlareModule
				*/

				//std::cout << "index " << i << " :" << key->FormatTypeName()->ToString() << std::endl;
				if (!key)
				{
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
