#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP

#include "api.hpp"
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <memory>

namespace cp
{

CONTEXTUALPLANNER_API
void unfoldMapWithSet(std::list<std::map<std::string, std::string>>& pOutMap,
                      const std::map<std::string, std::set<std::string>>& pInMap);


CONTEXTUALPLANNER_API
void applyNewParams(
    std::map<std::string, std::set<std::string>>& pParameters,
    std::map<std::string, std::set<std::string>>& pNewParameters);


CONTEXTUALPLANNER_API
std::string plusIntOrStr(
    const std::string& pNb1Str,
    const std::string& pNb2Str);

CONTEXTUALPLANNER_API
std::string minusIntOrStr(
    const std::string& pNb1Str,
    const std::string& pNb2Str);

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


template <typename T>
bool areUPtrEqual(const std::unique_ptr<T>& pPtr1,
                  const std::unique_ptr<T>& pPtr2)
{
  if (!pPtr1)
    return !pPtr2.operator bool();
  if (pPtr1 && pPtr2)
    return *pPtr1 == *pPtr2;
  return false;
}



}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
