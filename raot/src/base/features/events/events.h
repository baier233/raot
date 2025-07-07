#pragma once
#include <utils/types/math.hpp>
struct render_2d_event
{

};
#include <base/raot.h>
#define REGISTER_EVENT(eventType, callbackFunction) \
	raot::get().mm->registrations.push_back(raot::get().event_bus.register_handler<eventType>(this,&callbackFunction))