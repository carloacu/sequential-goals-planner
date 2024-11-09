#ifndef INCLUDE_CONTEXTUALPLANNER_API_HPP
#define INCLUDE_CONTEXTUALPLANNER_API_HPP

#include <prioritizedgoalsplanner/util/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(prioritizedgoalsplanner_EXPORTS)
# define CONTEXTUALPLANNER_API CONTEXTUALPLANNER_LIB_API_EXPORTS(prioritizedgoalsplanner)
#elif !defined(SWIG)
# define CONTEXTUALPLANNER_API CONTEXTUALPLANNER_LIB_API(prioritizedgoalsplanner)
#else
# define CONTEXTUALPLANNER_API
#endif

#endif // INCLUDE_CONTEXTUALPLANNER_API_HPP
