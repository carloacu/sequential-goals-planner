#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_API_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_API_HPP

#include <prioritizedgoalsplanner/util/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(prioritizedgoalsplanner_EXPORTS)
# define PRIORITIZEDGOALSPLANNER_API PRIORITIZEDGOALSPLANNER_LIB_API_EXPORTS(prioritizedgoalsplanner)
#elif !defined(SWIG)
# define PRIORITIZEDGOALSPLANNER_API PRIORITIZEDGOALSPLANNER_LIB_API(prioritizedgoalsplanner)
#else
# define PRIORITIZEDGOALSPLANNER_API
#endif

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_API_HPP
