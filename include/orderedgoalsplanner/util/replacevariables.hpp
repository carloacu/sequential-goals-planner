#ifndef INCLUDE_ORDEREDGOALSPLANNER_UTIL_REPLACEVARIABLES_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_UTIL_REPLACEVARIABLES_HPP

#include "api.hpp"
#include <map>
#include <string>

namespace ogp
{

ORDEREDGOALSPLANNER_API
void replaceVariables(std::string& pStr,
                      const std::map<std::string, std::string>& pVariablesToValue);


}

#endif // INCLUDE_ORDEREDGOALSPLANNER_UTIL_REPLACEVARIABLES_HPP
