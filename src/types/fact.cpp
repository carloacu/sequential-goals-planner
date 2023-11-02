#include <contextualplanner/types/fact.hpp>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <contextualplanner/types/factoptional.hpp>


namespace cp
{
namespace {

void _parametersToStr(std::string& pStr,
                      const std::vector<FactOptional>& pParameters)
{
  bool firstIteration = true;
  for (auto& param : pParameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param.toStr();
  }
}

bool _isInside(const std::string& pStr,
               const std::vector<std::string>* pEltsPtr)
{
  if (pEltsPtr == nullptr)
    return false;
  auto lastElt = pEltsPtr->end();
  return std::find(pEltsPtr->begin(), lastElt, pStr) != lastElt;
}

bool _isInside(const std::string& pStr,
               const std::map<std::string, std::set<std::string>>* pEltsPtr)
{
  if (pEltsPtr == nullptr)
    return false;
  return pEltsPtr->count(pStr) > 0;
}


}

const std::string Fact::anyValue = "*";
const FactOptional Fact::anyValueFact(Fact::anyValue);
std::string Fact::punctualPrefix = "~punctual~";
std::string Fact::unreachablePrefix = "~unreachable~";

Fact::Fact(const std::string& pStr,
           const char* pSeparatorPtr,
           bool* pIsFactNegatedPtr,
           std::size_t pBeginPos,
           std::size_t* pResPos)
  : name(),
    arguments(),
    value(),
    isValueNegated(false)
{
  auto resPos = fillFactFromStr(pStr, pSeparatorPtr, pBeginPos, pIsFactNegatedPtr);
  if (pResPos != nullptr)
    *pResPos = resPos;
}

Fact::~Fact()
{
}

bool Fact::operator<(const Fact& pOther) const
{
  if (name != pOther.name)
    return name < pOther.name;
  if (value != pOther.value)
    return value < pOther.value;
  if (isValueNegated != pOther.isValueNegated)
    return isValueNegated < pOther.isValueNegated;
  std::string paramStr;
  _parametersToStr(paramStr, arguments);
  std::string otherParamStr;
  _parametersToStr(otherParamStr, pOther.arguments);
  return paramStr < otherParamStr;
}

bool Fact::operator==(const Fact& pOther) const
{
  return name == pOther.name && arguments == pOther.arguments &&
      value == pOther.value && isValueNegated == pOther.isValueNegated;
}


bool Fact::areEqualWithoutValueConsideration(const Fact& pFact) const
{
  if (pFact.name != name ||
      pFact.arguments.size() != arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValueFact && *itOtherParam != anyValueFact)
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return true;
}


bool Fact::areEqualExceptAnyValues(const Fact& pOther,
                                   const std::map<std::string, std::set<std::string>>* pOtherFactArgumentsToConsiderAsAnyValuePtr,
                                   const std::vector<std::string>* pThisFactArgumentsToConsiderAsAnyValuePtr) const
{
  if (name != pOther.name || arguments.size() != pOther.arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pOther.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValueFact && *itOtherParam != anyValueFact &&
        !(!itParam->fact.arguments.empty() && itParam->fact.value != "" && _isInside(itParam->fact.name, pThisFactArgumentsToConsiderAsAnyValuePtr)) &&
        !(!itOtherParam->fact.arguments.empty() && itOtherParam->fact.value != "" && _isInside(itOtherParam->fact.name, pOtherFactArgumentsToConsiderAsAnyValuePtr)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  if (!(value == pOther.value || value == anyValue || pOther.value == anyValue ||
        _isInside(value, pThisFactArgumentsToConsiderAsAnyValuePtr) || _isInside(pOther.value, pOtherFactArgumentsToConsiderAsAnyValuePtr)))
    return isValueNegated != pOther.isValueNegated;

  return isValueNegated == pOther.isValueNegated;
}


bool Fact::isPunctual() const
{
  return name.compare(0, punctualPrefix.size(), punctualPrefix) == 0;
}

bool Fact::isUnreachable() const
{
  return name.compare(0, unreachablePrefix.size(), unreachablePrefix) == 0;
}


std::string Fact::tryToExtractArgumentFromExample(
    const std::string& pArgument,
    const Fact& pOther) const
{
  if (name != pOther.name ||
      isValueNegated != pOther.isValueNegated ||
      arguments.size() != pOther.arguments.size())
    return "";

  std::string res;
  if (value == pArgument)
    res = pOther.value;
   else if (value != pOther.value)
    return "";

  auto itParam = arguments.begin();
  auto itOtherParam = pOther.arguments.begin();
  while (itParam != arguments.end())
  {
    if (itParam->fact.name == pArgument)
      res = itOtherParam->fact.name;
    else if (itParam->fact.name != itOtherParam->fact.name)
      return "";

    auto subRes = itParam->fact.tryToExtractArgumentFromExample(pArgument, itOtherParam->fact);
    if (subRes != "")
      return subRes;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}


void Fact::replaceArguments(const std::map<std::string, std::string>& pCurrentArgumentsToNewArgument)
{
  auto itValueParam = pCurrentArgumentsToNewArgument.find(value);
  if (itValueParam != pCurrentArgumentsToNewArgument.end())
    value = itValueParam->second;

  for (auto& currParam : arguments)
  {
    auto& currFactParam = currParam.fact;
    if (currFactParam.value.empty() && currFactParam.arguments.empty())
    {
      auto itValueParam = pCurrentArgumentsToNewArgument.find(currFactParam.name);
      if (itValueParam != pCurrentArgumentsToNewArgument.end())
        currFactParam.name = itValueParam->second;
    }
    else
    {
      currFactParam.replaceArguments(pCurrentArgumentsToNewArgument);
    }
  }
}

void Fact::replaceArguments(const std::map<std::string, std::set<std::string>>& pCurrentArgumentsToNewArgument)
{
  auto itValueParam = pCurrentArgumentsToNewArgument.find(value);
  if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
    value = *itValueParam->second.begin();

  for (auto& currParam : arguments)
  {
    auto& currFactParam = currParam.fact;
    if (currFactParam.value.empty() && currFactParam.arguments.empty())
    {
      auto itValueParam = pCurrentArgumentsToNewArgument.find(currFactParam.name);
      if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
        currFactParam.name = *itValueParam->second.begin();
    }
    else
    {
      currFactParam.replaceArguments(pCurrentArgumentsToNewArgument);
    }
  }
}

std::string Fact::toStr() const
{
  std::string res = name;
  if (!arguments.empty())
  {
    res += "(";
    _parametersToStr(res, arguments);
    res += ")";
  }
  if (isValueNegated)
    res += "!=" + value;
  else if (!value.empty())
    res += "=" + value;
  return res;
}


Fact Fact::fromStr(const std::string& pStr,
                   bool* pIsFactNegatedPtr)
{
  return Fact(pStr, nullptr, pIsFactNegatedPtr);
}


std::size_t Fact::fillFactFromStr(
    const std::string& pStr,
    const char* pSeparatorPtr,
    std::size_t pBeginPos,
    bool* pIsFactNegatedPtr)
{
  static const char separatorOfParameters = ',';
  std::size_t pos = pBeginPos;
  while (pos < pStr.size())
  {
    if (pStr[pos] == ' ')
    {
      ++pos;
      continue;
    }
    if (pStr[pos] == '!')
    {
      if (pIsFactNegatedPtr != nullptr)
        *pIsFactNegatedPtr = true;
      ++pos;
      continue;
    }
    if ((pSeparatorPtr != nullptr && pStr[pos] == *pSeparatorPtr) || pStr[pos] == ')')
      return pos;

    bool insideParenthesis = false;
    auto beginPos = pos;
    while (pos < pStr.size())
    {
      if (!insideParenthesis && ((pSeparatorPtr != nullptr && pStr[pos] == *pSeparatorPtr) || pStr[pos] == ' ' || pStr[pos] == ')'))
        break;
      if (pStr[pos] == '!')
      {
        isValueNegated = true;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        continue;
      }
      if (pStr[pos] == '(' || pStr[pos] == separatorOfParameters)
      {
        insideParenthesis = true;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        arguments.emplace_back(pStr, &separatorOfParameters, pos, &pos);
        beginPos = pos;
        continue;
      }
      if (pStr[pos] == ')' || pStr[pos] == '=')
      {
        insideParenthesis = false;
        if (name.empty())
          name = pStr.substr(beginPos, pos - beginPos);
        ++pos;
        beginPos = pos;
        continue;
      }
      ++pos;
    }
    if (name.empty())
      name = pStr.substr(beginPos, pos - beginPos);
    else
      value = pStr.substr(beginPos, pos - beginPos);
  }
  return pos;
}


bool Fact::replaceSomeArgumentsByAny(const std::vector<std::string>& pArgumentsToReplace)
{
  bool res = false;
  for (const auto& currParam : pArgumentsToReplace)
  {
    for (auto& currFactParam : arguments)
    {
      if (currFactParam == currParam)
      {
        currFactParam = anyValueFact;
        res = true;
      }
    }
    if (value == currParam)
    {
      value = anyValue;
      res = true;
    }
  }
  return res;
}


bool Fact::isInOtherFacts(const std::set<Fact>& pOtherFacts,
                          bool pParametersAreForTheFact,
                          std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                          const std::map<std::string, std::set<std::string>>* pParametersPtr,
                          bool* pTriedToModifyParametersPtr) const
{
  bool res = false;
  for (const auto& currOtherFact : pOtherFacts)
    if (isInOtherFact(currOtherFact, pParametersAreForTheFact, pNewParametersPtr, pParametersPtr,
                 pTriedToModifyParametersPtr))
      res = true;
  return res;
}


bool Fact::isInOtherFact(const Fact& pOtherFact,
                         bool pParametersAreForTheFact,
                         std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                         const std::map<std::string, std::set<std::string>>* pParametersPtr,
                         bool* pTriedToModifyParametersPtr) const
{
  if (pOtherFact.name != name ||
      pOtherFact.arguments.size() != arguments.size())
    return false;


  auto doesItMatch = [&](const std::string& pFactValue, const std::string& pValueToLookFor) {
    if (pFactValue == pValueToLookFor)
      return true;

    if (pParametersPtr != nullptr)
    {
      auto itParam = pParametersPtr->find(pFactValue);
      if (itParam != pParametersPtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        if (pNewParametersPtr != nullptr)
          (*pNewParametersPtr)[pFactValue].insert(pValueToLookFor);
        else if (pTriedToModifyParametersPtr != nullptr)
          *pTriedToModifyParametersPtr = true;
        return true;
      }
    }
    return false;
  };

  {
    bool doesParametersMatches = true;
    auto itFactParameters = pOtherFact.arguments.begin();
    auto itLookForParameters = arguments.begin();
    while (itFactParameters != pOtherFact.arguments.end())
    {
      if (*itFactParameters != *itLookForParameters)
      {
        if (!itFactParameters->fact.arguments.empty() ||
            !itFactParameters->fact.value.empty() ||
            !itLookForParameters->fact.arguments.empty() ||
            !itLookForParameters->fact.value.empty() ||
            (!pParametersAreForTheFact && !doesItMatch(itFactParameters->fact.name, itLookForParameters->fact.name)) ||
            (pParametersAreForTheFact && !doesItMatch(itLookForParameters->fact.name, itFactParameters->fact.name)))
          doesParametersMatches = false;
      }
      ++itFactParameters;
      ++itLookForParameters;
    }
    if (!doesParametersMatches)
      return false;
  }

  if (pParametersAreForTheFact)
  {
    if (doesItMatch(value, pOtherFact.value))
      return pOtherFact.isValueNegated == isValueNegated;
  }
  else
  {
    if (doesItMatch(pOtherFact.value, value))
      return pOtherFact.isValueNegated == isValueNegated;
  }
  return pOtherFact.isValueNegated != isValueNegated;
}


void Fact::replaceFactInArguments(const Fact& pCurrentFact,
                                  const Fact& pNewFact)
{
  for (auto& currParameter : arguments)
  {
    if (currParameter.fact == pCurrentFact)
      currParameter.fact = pNewFact;
    else
      currParameter.fact.replaceFactInArguments(pCurrentFact, pNewFact);
  }
}


} // !cp
