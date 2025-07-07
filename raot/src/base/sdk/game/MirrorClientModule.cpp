#include "MirrorClientModule.h"

auto MirrorClientModule::get_instance() -> MirrorClientModule*
{
	auto instances = cached->FindObjectsByType<MirrorClientModule*>();
	if (instances.empty())
	{
		return nullptr;
	}
	return instances.front();
}

std::vector<MirrorClientObject*> MirrorClientModule::get_clients()
{
	static int32_t offset = field_manifest->offset;
	std::vector<MirrorClientObject*> clients;
	//auto dictionary = reinterpret_cast<II::Dictionary<II::Int32*, MirrorClientObject*>*>(reinterpret_cast<uint8_t*>(this) + offset);
	II::Dictionary<II::Int32*, MirrorClientObject*>* dictionary = nullptr;
	//auto dictionary = field_manifest_var.Get(this);
	field_manifest->GetValue(this, &dictionary);
	//std::cout << "dictionary :" << dictionary << std::endl;

	if (dictionary && dictionary->iCount && dictionary->GetEntry()) {
		for (size_t i = 0; i < dictionary->iCount; i++)
		{
			clients.push_back(dictionary->GetValueByIndex(i));
		}
	}

	return clients;
}

auto MirrorClientModule::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorClientModule")) {
		cached = pClass;
		field_manifest = pClass->Get<IF>("<Manifest>k__BackingField");
		field_manifest_var.Init(field_manifest);
	}

}
