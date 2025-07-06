#pragma once
#include <iostream>
#include <vector>
#include <array>

namespace values {
	enum type {
		BOOL,
		INT,
		FLOAT,
		LIST,
		COLOR
	};

	class value {
	public:

		inline value(const std::string& name) {
			m_name = name;
			m_desc = name;
			this->m_value = 0;
		}

		inline value(const std::string& name, const std::string& desc) {
			m_name = name;
			m_desc = desc;
			this->m_value = 0;
		}

		inline std::string get_name() {
			return m_name;
		}

		inline std::string get_desc() {
			return m_desc;
		}

		inline void set_value(int64_t value) {
			m_value = value;
		}

		inline int64_t* get_ptr() {
			return (int64_t*)&m_value;
		}

		inline virtual ~value() {

		}


	protected:
		int64_t m_value;
		std::string m_name;
		std::string m_desc;
	};

	class number_value : public value {
	public:
		inline number_value(const std::string& name, int64_t value, int64_t min, int64_t max) : value(name) {
			set_value(value);
			m_min = min;
			m_max = max;
		}
		inline number_value(const std::string& name, const std::string& desc, int64_t value, int64_t min, int64_t max) : value(name, desc) {
			set_value(value);
			m_min = min;
			m_max = max;
		}

		inline int64_t get_value() { return m_value; }

		inline int64_t get_min() { return m_min; }
		inline int64_t get_max() { return m_max; }
	private:
		int64_t m_min, m_max;
	};

	class bool_value : public value {
	public:
		inline bool_value(const std::string& name, bool enable) : value(name) {
			set_value(enable);
		}

		inline bool_value(const std::string& name, const std::string& desc, bool enable) : value(name, desc) {
			set_value(enable);
		}

		inline bool get_value() {
			return m_value;
		}
		inline bool* get_value_ptr()
		{
			return (bool*)get_ptr();
		}
	};
	class float_value : public value {
	public:
		inline float_value(const std::string& name, float value, float min, float max) : value(name) {

			set_value(*(int64_t*)&value); //对float的值进行特殊处理，以免被编译器视为类型转换
			m_min = min;
			m_max = max;
		}

		inline float_value(const std::string& name, const std::string& desc, float value, float min, float max) : value(name, desc) {

			set_value(*(int64_t*)&value); //对float的值进行特殊处理，以免被编译器视为类型转换
			m_min = min;
			m_max = max;
		}

		inline float get_value() {
			return *(float*)&m_value; //对float的值进行特殊处理，以免被编译器视为类型转换
		}

		inline float* get_value_ptr() {
			return (float*)get_ptr();
		}

		inline float get_min() { return m_min; }
		inline float get_max() { return m_max; }

	protected:
		float m_min, m_max;
	};

	class mode_value : public value {
	public:
		//desc
		inline mode_value(const std::string& name, std::vector<int> modes, std::vector<std::string> desc, int value) : value(name) {
			set_value(value);
			m_modes = modes;
			m_desc = desc;
		}

		inline mode_value(const std::string& name, const std::string& module_desc, std::vector<int> modes, const std::vector<std::string>& desc, int value) : value(name, module_desc) {
			set_value(value);
			m_modes = modes;
			m_desc = desc;
		}

		inline int get_value() {
			return m_value;
		}

		inline std::vector<int> get_modes() {
			return m_modes;
		};

		inline std::vector<std::string> get_descs() {
			return m_desc;
		}

		inline int* get_value_ptr()
		{
			return (int*)get_ptr();
		}

	private:
		std::vector<int> m_modes;
		std::vector<std::string> m_desc;
	};

	class color_value : public value {
	public:
		inline color_value(const std::string& name, float colorArray[4]) : value(name) {
			m_colorArray = std::array<float, 4>{ colorArray[0], colorArray[1], colorArray[2], colorArray[3] };
		}
		inline color_value(const std::string& name, const std::string& desc, float colorArray[4]) : value(name, desc) {
			m_colorArray = std::array<float, 4>{ colorArray[0], colorArray[1], colorArray[2], colorArray[3] };
		}

		inline float* get_value() {

			return m_colorArray.data();
		}

		inline float* get_value_ptr() {
			return m_colorArray.data();
		}

	private:
		std::array<float, 4> m_colorArray;
	};


}


#define DEFINE_FLOAT_VALUE(VAR_NAME,NAME,DESC,DEFAULT,MIN,MAX) std::shared_ptr<values::float_value> VAR_NAME = std::make_unique<values::float_value>((NAME), (DESC), DEFAULT, MIN, MAX)

#define DEFINE_BOOL_VALUE(VAR_NAME,NAME,DESC,DEFAULT) std::shared_ptr<values::bool_value> VAR_NAME = std::make_unique<values::bool_value>((NAME), (DESC), DEFAULT)