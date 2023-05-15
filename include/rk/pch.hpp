#ifndef RK_PCH_HPP
#define RK_PCH_HPP

#include <vector>
#include <cstdint>
#include <cmath>
#ifdef HAS_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif
#ifdef HAS_DEBUG_LOG_TOOLS
#include "dbg/log.hpp"
#endif
#ifdef HAS_PROFILE_TOOLS
#include "perf/perf.hpp"
#endif
#include "rk/core.hpp"

#endif