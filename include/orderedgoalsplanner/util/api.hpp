#ifndef INCLUDE_ORDEREDGOALSPLANNER_API_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_API_HPP

#include <orderedgoalsplanner/util/exportsymbols/macro.hpp>

#if !defined(SWIG) && defined(orderedgoalsplanner_EXPORTS)
# define ORDEREDGOALSPLANNER_API ORDEREDGOALSPLANNER_LIB_API_EXPORTS(orderedgoalsplanner)
#elif !defined(SWIG)
# define ORDEREDGOALSPLANNER_API ORDEREDGOALSPLANNER_LIB_API(orderedgoalsplanner)
#else
# define ORDEREDGOALSPLANNER_API
#endif

#endif // INCLUDE_ORDEREDGOALSPLANNER_API_HPP
