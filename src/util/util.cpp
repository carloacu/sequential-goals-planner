#include <contextualplanner/util/util.hpp>
#include <cctype> // For isdigit()
#include <sstream>
#include <contextualplanner/types/entity.hpp>
#include <contextualplanner/types/parameter.hpp>

namespace cp
{
bool CONTEXTUALPLANNER_DEBUG_FOR_TESTS = false;  // Define the variable and initialize it

namespace
{

void _unfoldMapWithSet(std::list<std::map<Parameter, Entity>>& pOutMap,
                       std::map<Parameter, std::set<Entity>>& pInMap)
{
  if (pInMap.size() == 1)
  {
    auto itFirstElt = pInMap.begin();
    for (auto& currValue : itFirstElt->second)
      pOutMap.emplace_back(std::map<Parameter, Entity>{{itFirstElt->first, currValue}});
    return;
  }


  while (!pInMap.empty())
  {
    auto itFirstElt = pInMap.begin();
    auto key = itFirstElt->first;
    auto values = std::move(itFirstElt->second);
    pInMap.erase(itFirstElt);

    std::list<std::map<Parameter, Entity>> subRes;
    unfoldMapWithSet(subRes, pInMap);

    for (auto& currValue : values)
    {
      auto newRes = subRes;
      for (auto& currSubResValue : newRes)
      {
        currSubResValue.emplace(key, currValue);
        pOutMap.emplace_back(std::move(currSubResValue));
      }
    }
  }
}

}


bool isNumber(const std::string& str) {
    for (char const &c : str)
        if (!std::isdigit(c))
          return false;
    return !str.empty(); // Ensure it's not an empty string
}

void unfoldMapWithSet(std::list<std::map<Parameter, Entity>>& pOutMap,
                      const std::map<Parameter, std::set<Entity>>& pInMap)
{
  auto inMap = pInMap;
  _unfoldMapWithSet(pOutMap, inMap);
}


void applyNewParams(
    std::map<Parameter, std::set<Entity>>& pParameters,
    std::map<Parameter, std::set<Entity>>& pNewParameters)
{
  for (auto& currNewParam : pNewParameters)
    pParameters[currNewParam.first] = std::move(currNewParam.second);
}



std::optional<Entity> plusIntOrStr(const std::optional<Entity>& pNb1,
                                   const std::optional<Entity>& pNb2)
{
  if (!pNb1 || !pNb2 || pNb1->type != pNb2->type)
    return {};
  try
  {
    int nb1 = lexical_cast<int>(pNb1->value);
    int nb2 = lexical_cast<int>(pNb2->value);
    std::stringstream ss;
    ss << nb1 + nb2;
    return Entity(ss.str(), pNb1->type);
  } catch (...) {}
  return Entity(pNb1->value + pNb2->value, pNb1->type);
}


std::optional<Entity> minusIntOrStr(const std::optional<Entity>& pNb1,
                                    const std::optional<Entity>& pNb2)
{
  if (!pNb1 || !pNb2 || pNb1->type != pNb2->type)
    return {};
  try
  {
    int nb1 = lexical_cast<int>(pNb1->value);
    int nb2 = lexical_cast<int>(pNb2->value);
    std::stringstream ss;
    ss << nb1 - nb2;
    return Entity(ss.str(), pNb1->type);
  } catch (...) {}
  return Entity(pNb1->value + "-" + pNb2->value, pNb1->type);
}


bool compIntNb(
    const std::string& pNb1Str,
    int pNb2,
    bool pBoolSuperiorOrInferior)
{
  try
  {
    int nb1 = lexical_cast<int>(pNb1Str);
    if (pBoolSuperiorOrInferior)
      return nb1 > pNb2;
    else
      return nb1 < pNb2;
  } catch (...) {}
  return false;
}

std::string incrementLastNumberUntilAConditionIsSatisfied(
    const std::string& pStr,
    const std::function<bool(const std::string&)>& pCondition)
{
  if (pCondition(pStr))
    return pStr;

  std::string base = pStr;
  std::size_t versionNb = 2;

  auto posUnderscore = pStr.find('_');
  if (posUnderscore != std::string::npos)
  {
    auto nbBeginOfPos = posUnderscore + 1;
    if (pStr.size() > nbBeginOfPos)
    {
      try
      {
        versionNb = lexical_cast<std::size_t>(pStr.substr(nbBeginOfPos, pStr.size() - nbBeginOfPos));
        base = pStr.substr(0, posUnderscore);
      } catch (...) {}
    }
  }

  for (std::size_t i = 0; i < 1000000; ++i)
  {
    std::stringstream currentSs;
    currentSs << base << "_" << versionNb;
    auto currentStr = currentSs.str();
    if (pCondition(currentStr))
      return currentStr;
    ++versionNb;
  }
  throw std::runtime_error("incrementLastNumberUntilAConditionIsSatisfied(" + pStr + ") cannot find a condition statisfied");
}



void split(std::vector<std::string>& pStrs,
           const std::string& pStr,
           const std::string& pSeparator)
{
  std::string::size_type lastPos = 0u;
  std::string::size_type pos = lastPos;
  std::size_t separatorSize = pSeparator.size();
  while ((pos = pStr.find(pSeparator, pos)) != std::string::npos)
  {
    pStrs.emplace_back(pStr.substr(lastPos, pos - lastPos));
    pos += separatorSize;
    lastPos = pos;
  }
  pStrs.emplace_back(pStr.substr(lastPos, pStr.size() - lastPos));
}


// trim from start (in place)
void ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
    return !std::isspace(ch);
  }));
}


// trim from end (in place)
void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}


// trim from end (in place)
void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}



}
