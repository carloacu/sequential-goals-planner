#include <prioritizedgoalsplanner/util/replacevariables.hpp>
#include <prioritizedgoalsplanner/util/arithmeticevaluator.hpp>

namespace pgp
{


void replaceVariables(std::string& pStr,
                      const std::map<std::string, std::string>& pVariablesToValue)
{
  // replace variable
  auto currentPos = pStr.find_first_of("${");
  while (currentPos != std::string::npos)
  {
    auto beginOfVarName = currentPos + 2;
    auto endVarPos = pStr.find("}", beginOfVarName);
    if (endVarPos != std::string::npos)
    {
      auto varParam = pStr.substr(beginOfVarName, endVarPos - beginOfVarName);

      auto it = pVariablesToValue.find(varParam);
      if (it != pVariablesToValue.end())
      {
        auto& varValue = it->second;
        pStr.replace(currentPos, endVarPos - currentPos + 1, varValue);
        currentPos += varValue.size();
      }
      else
      {
        currentPos = endVarPos;
      }

    }
    currentPos = pStr.find_first_of("${", currentPos);
  }

  // evalute expressions
  currentPos = pStr.find("`");
  while (currentPos != std::string::npos)
  {
    auto beginOfExp = currentPos + 1;
    auto endExpPos = pStr.find("`", beginOfExp);
    if (endExpPos == std::string::npos)
      break;
    pStr.replace(currentPos, endExpPos - currentPos + 1, evaluteToStr(pStr, beginOfExp));
    currentPos = endExpPos + 1;
    currentPos = pStr.find("`", currentPos);
  }
}

}
