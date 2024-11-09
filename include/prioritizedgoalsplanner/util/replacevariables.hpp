#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_REPLACEVARIABLES_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_REPLACEVARIABLES_HPP

#include "api.hpp"
#include <map>
#include <string>

namespace cp
{

CONTEXTUALPLANNER_API
void replaceVariables(std::string& pStr,
                      const std::map<std::string, std::string>& pVariablesToValue);


}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_REPLACEVARIABLES_HPP
