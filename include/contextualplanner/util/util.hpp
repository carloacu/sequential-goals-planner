#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP

#include "api.hpp"
#include <list>
#include <map>
#include <set>
#include <string>

namespace cp
{

CONTEXTUALPLANNER_API
void unfoldMapWithSet(std::list<std::map<std::string, std::string>>& pOutMap,
                      const std::map<std::string, std::set<std::string>>& pInMap);


CONTEXTUALPLANNER_API
void applyNewParams(
    std::map<std::string, std::set<std::string>>& pParameters,
    std::map<std::string, std::set<std::string>>& pNewParameters);


}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
