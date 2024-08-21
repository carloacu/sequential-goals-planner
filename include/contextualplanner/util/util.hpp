#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP

#include "api.hpp"
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <memory>

namespace cp
{
struct Entity;
struct Parameter;


extern bool CONTEXTUALPLANNER_DEBUG_FOR_TESTS;

CONTEXTUALPLANNER_API
void unfoldMapWithSet(std::list<std::map<Parameter, Entity>>& pOutMap,
                      const std::map<Parameter, std::set<Entity>>& pInMap);


CONTEXTUALPLANNER_API
void applyNewParams(std::map<Parameter, std::set<Entity>>& pParameters,
                    std::map<Parameter, std::set<Entity>>& pNewParameters);


CONTEXTUALPLANNER_API
std::optional<std::string> plusIntOrStr(const std::optional<Entity>& pNb1,
                                        const std::optional<Entity>& pNb2);

CONTEXTUALPLANNER_API
std::optional<std::string> minusIntOrStr(const std::optional<Entity>& pNb1,
                                         const std::optional<Entity>& pNb2);

CONTEXTUALPLANNER_API
bool compIntNb(
    const std::string& pNb1Str,
    int pNb2,
    bool pBoolSuperiorOrInferior);

CONTEXTUALPLANNER_API
std::string incrementLastNumberUntilAConditionIsSatisfied(
    const std::string& pStr,
    const std::function<bool(const std::string&)>& pCondition);

CONTEXTUALPLANNER_API
void split(std::vector<std::string>& pStrs,
           const std::string& pStr,
           const std::string& pSeparator);


void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

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



template <typename T>
std::list<T> mergeTwoLists(const std::list<T>& pList1,
                           const std::list<T>& pList2)
{
  std::list<T> res = pList1;
  res.insert(res.end(), pList2.begin(), pList2.end());
  return res;
}


template <typename T>
std::list<T> mergeTwoListsWithNoDoubleEltCheck(const std::list<T>& pList1,
                                               const std::list<T>& pList2)
{
  std::list<T> res = pList1;
  for (auto& currElt2 : pList2)
  {
    auto it = std::find(pList1.begin(), pList1.end(), currElt2);
    if (it == pList1.end())
      res.insert(res.end(), currElt2);
  }
  return res;
}


template <typename T>
std::list<T> intersectTwoLists(const std::list<T>& pList1,
                               const std::list<T>& pList2)
{
  std::list<T> intersectionList;
  for (auto& elem : pList1) {
      if (std::find(pList2.begin(), pList2.end(), elem) != pList2.end()) {
          intersectionList.push_back(elem);
      }
  }
  return intersectionList;
}



}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_UTIL_HPP
