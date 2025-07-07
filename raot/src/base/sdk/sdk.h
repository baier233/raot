#pragma once
#include <UnityResolve.hpp>

using I = UnityResolve;
using IM = UnityResolve::Method;
using IC = UnityResolve::Class;
using IT = UnityResolve::Type;
using IF = UnityResolve::Field;
using IA = UnityResolve::Assembly;
using II = UnityResolve::UnityType;

namespace sdk {
	bool init(bool dump_sdk = false);
	bool attach();
}