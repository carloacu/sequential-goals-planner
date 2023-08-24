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



std::string add(
    const std::string& pNb1Str,
    int pNb2)
{
  try {
    int nb1 = lexical_cast<int>(pNb1Str);
    std::stringstream ss;
    ss << nb1 + pNb2;
    return ss.str();
  }  catch (...) {}
  return pNb1Str;
}


}
