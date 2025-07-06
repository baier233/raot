#pragma once
#include <utils/Singleton.hpp>
#include <httplib/httplib.h>
namespace webui {
	class server : public singleton<server>
	{
	public:
		void start(int port);
		void detach();
	private:
		httplib::Server srv;
		int port = 15935;
		void listen_thread();
	};
}
