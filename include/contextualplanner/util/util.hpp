#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP

#include "api.hpp"
#include <list>
#include <map>
#include <set>
#include <stdexcept>
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


std::string add(
    const std::string& pNb1Str,
    int pNb2);


template <typename T>
T lexical_cast(const std::string& pStr)
{
  bool firstChar = true;
  for (const auto& currChar : pStr)
  {
    if ((currChar < '0' || currChar > '9') &&
        !(firstChar && currChar == '-'))
      throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
    firstChar = false;
  }
  return atoi(pStr.c_str());
}


}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
