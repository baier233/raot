#pragma once
#include <base/features/values/value.h>

enum class category : unsigned int {
	COMBAT = 0,
	MOVEMENT,
	PLAYER,
	VISUAL,
	WORLD,
	SETTING
};
inline std::wstring getCategoryDisplayName(category category) {
	switch (category) {
	case category::COMBAT:
		return L"Combat";
	case category::MOVEMENT:
		return L"Movement";
	case category::PLAYER:
		return L"Player";
	case category::WORLD:
		return L"World";
	case category::VISUAL:
		return L"Visual";
	case category::SETTING:
		return L"Setting";
	default:
		return L"UNKNOWN";
	}
}

inline std::wstring getCategoryIcon(category category) {
	switch (category) {
	case category::COMBAT:
		return L"\uE9DB";
	case category::MOVEMENT:
		return L"\uEB34";
	case category::PLAYER:
		return L"\uEBAD";
	case category::WORLD:
		return L"\uEBB1";
	case category::VISUAL:
		return L"\uEAE1";
	case category::SETTING:
		return L"\uEBB1";
	default:
		return L"UNKNOWN";
	}
}
class abstract_module
{
protected:
	std::vector<std::pair<values::type, std::shared_ptr<values::value>>> values;
private:
	std::string name = "";
	std::string desc = "";
	int key = 0;
	category _category = category::COMBAT;
	bool i_toggle = false;
public:

	inline void add_value(values::type type, const std::shared_ptr<values::value>& value)
	{
		values.push_back({ type, value });
	}

	inline auto get_values() -> auto {
		return this->values;
	}

	category get_category();

	void toggle();

	void on_toggle();

	std::string get_name();

	std::string get_desc();

	int get_key();

	bool get_toggle();

	void set_toggle(bool _new);
	void set_key(int _new);

	virtual void on_enable() = 0;
	virtual void on_disable() = 0;
	virtual void on_update() = 0;


	abstract_module(const char* name, category cate);
	abstract_module(const char* name, category cate, std::string desc);
	abstract_module(const char* name, category cate, int k);
	abstract_module(const char* name, category cate, std::string desc, int k);

	virtual ~abstract_module() = default;
	void set_toggle_async(bool _new);
	abstract_module(abstract_module&&) = delete;
};

template<class T >
class instance_module : public abstract_module {
public:
	using abstract_module::abstract_module;
	static inline T& instance() { static T instance_; return instance_; }
};