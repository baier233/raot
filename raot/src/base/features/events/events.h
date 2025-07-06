#pragma once
#include <utils/types/math.hpp>

#include <base/raot.h>
#define REGISTER_EVENT(eventType, callbackFunction) \
	raot::get().mm->registrations.push_back(raot::get().event_bus.register_handler<eventType>(this,&callbackFunction))