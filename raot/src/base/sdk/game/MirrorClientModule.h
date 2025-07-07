#pragma once
#include "MirrorClientObject.h"
class MirrorClientModule : public II::MonoBehaviour
{
public:
	static inline I::Class* cached;
	static inline I::Field* field_manifest;
	static inline IF::Variable<II::Dictionary<II::Int32*, MirrorClientObject*>, MirrorClientModule> field_manifest_var;
	static auto get_instance() -> MirrorClientModule*;
	std::vector<MirrorClientObject*> get_clients();
	static auto init() -> void;
};

