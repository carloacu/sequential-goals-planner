#include <contextualplanner/util/util.hpp>
#include <sstream>

namespace cp
{

namespace
{

void _unfoldMapWithSet(std::list<std::map<std::string, std::string>>& pOutMap,
                       std::map<std::string, std::set<std::string>>& pInMap)
{
  if (pInMap.size() == 1)
  {
    auto itFirstElt = pInMap.begin();
    for (auto& currValue : itFirstElt->second)
      pOutMap.emplace_back(std::map<std::string, std::string>{{itFirstElt->first, currValue}});
    return;
  }


  while (!pInMap.empty())
  {
    auto itFirstElt = pInMap.begin();
    auto key = itFirstElt->first;
    auto values = std::move(itFirstElt->second);
    pInMap.erase(itFirstElt);

    std::list<std::map<std::string, std::string>> subRes;
    unfoldMapWithSet(subRes, pInMap);

    for (auto& currValue : values)
    {
      auto newRes = subRes;
      for (auto& currSubResValue : newRes)
      {
        currSubResValue[key] = currValue;
        pOutMap.emplace_back(std::move(currSubResValue));
      }
    }
  }
}

}


void unfoldMapWithSet(std::list<std::map<std::string, std::string>>& pOutMap,
                      const std::map<std::string, std::set<std::string>>& pInMap)
{
  auto inMap = pInMap;
  _unfoldMapWithSet(pOutMap, inMap);
}


void applyNewParams(
    std::map<std::string, std::set<std::string>>& pParameters,
    std::map<std::string, std::set<std::string>>& pNewParameters)
{
  for (auto& currNewParam : pNewParameters)
    pParameters[currNewParam.first] = std::move(currNewParam.second);
}



std::string plusIntOrStr(
    const std::string& pNb1Str,
    const std::string& pNb2Str)
{
  try
  {
    int nb1 = lexical_cast<int>(pNb1Str);
    int nb2 = lexical_cast<int>(pNb2Str);
    std::stringstream ss;
    ss << nb1 + nb2;
    return ss.str();
  } catch (...) {}
  return pNb1Str + pNb2Str;
}


std::string minusIntOrStr(
    const std::string& pNb1Str,
    const std::string& pNb2Str)
{
  try
  {
    int nb1 = lexical_cast<int>(pNb1Str);
    int nb2 = lexical_cast<int>(pNb2Str);
    std::stringstream ss;
    ss << nb1 - nb2;
    return ss.str();
  } catch (...) {}
  return pNb1Str + "-" + pNb2Str;
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



}
