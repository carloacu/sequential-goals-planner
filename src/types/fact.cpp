#include <contextualplanner/types/fact.hpp>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <optional>
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
const std::string Fact::undefinedValue = "undefined";
std::string Fact::punctualPrefix = "~punctual~";

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

bool Fact::areEqualWithoutAnArgConsideration(const Fact& pFact,
                                             const std::string& pArgToIgnore) const
{
  if (pFact.name != name ||
      pFact.arguments.size() != arguments.size() ||
      pFact.value != value)
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValueFact && *itOtherParam != anyValueFact &&
        *itParam != pArgToIgnore)
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return true;
}



bool Fact::areEqualExceptAnyValues(const Fact& pOther,
                                   const std::map<std::string, std::set<std::string>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                   const std::map<std::string, std::set<std::string>>* pOtherFactParametersToConsiderAsAnyValuePtr2,
                                   const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr) const
{
  if (name != pOther.name || arguments.size() != pOther.arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pOther.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValueFact && *itOtherParam != anyValueFact &&
        !(_isInside(itParam->fact.name, pThisFactParametersToConsiderAsAnyValuePtr)) &&
        !(_isInside(itOtherParam->fact.name, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(itOtherParam->fact.name, pOtherFactParametersToConsiderAsAnyValuePtr2)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  if (!(value == pOther.value || value == anyValue || pOther.value == anyValue ||
        _isInside(value, pThisFactParametersToConsiderAsAnyValuePtr) ||
        _isInside(pOther.value, pOtherFactParametersToConsiderAsAnyValuePtr) ||
        _isInside(pOther.value, pOtherFactParametersToConsiderAsAnyValuePtr2)))
    return isValueNegated != pOther.isValueNegated;

  return isValueNegated == pOther.isValueNegated;
}


bool Fact::isPunctual() const
{
  return name.compare(0, punctualPrefix.size(), punctualPrefix) == 0;
}


bool Fact::hasArgumentOrValue(
    const std::string& pArgumentOrValue) const
{
  if (value == pArgumentOrValue)
    return true;

  auto itParam = arguments.begin();
  while (itParam != arguments.end())
  {
    if (itParam->fact.name == pArgumentOrValue)
      return true;
    if (itParam->fact.hasArgumentOrValue(pArgumentOrValue))
      return true;
    ++itParam;
  }
  return false;
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

    auto subRes = itParam->fact.tryToExtractArgumentFromExample(pArgument, itOtherParam->fact);
    if (subRes != "")
      return subRes;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}


bool Fact::isPatternOf(
    const std::map<std::string, std::set<std::string>>& pPossibleArguments,
    const Fact& pFactExample) const
{
  if (name != pFactExample.name ||
      isValueNegated != pFactExample.isValueNegated ||
      arguments.size() != pFactExample.arguments.size())
    return false;

  auto isOk = [&](const std::string& pPatternVal,
                  const std::string& pExempleVal) {
    auto itVal = pPossibleArguments.find(pPatternVal);
    if (itVal != pPossibleArguments.end())
    {
      if (!itVal->second.empty() &&
          itVal->second.count(pExempleVal) == 0)
        return false;
    }
    return true;
  };

  if (!isOk(value, pFactExample.value))
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFactExample.arguments.begin();
  while (itParam != arguments.end())
  {
    if (!isOk(itParam->fact.name, itOtherParam->fact.name))
      return false;
    ++itParam;
    ++itOtherParam;
  }
  return true;
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
                          std::map<std::string, std::set<std::string>>* pParametersToModifyInPlacePtr,
                          bool* pTriedToModifyParametersPtr) const
{
  bool res = false;
  for (const auto& currOtherFact : pOtherFacts)
    if (isInOtherFact(currOtherFact, pParametersAreForTheFact, pNewParametersPtr, pParametersPtr,
                 pParametersToModifyInPlacePtr, pTriedToModifyParametersPtr))
      res = true;
  return res;
}


bool Fact::isInOtherFactsMap(const std::map<std::string, std::set<Fact>>& pOtherFacts,
                             bool pParametersAreForTheFact,
                             std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                             const std::map<std::string, std::set<std::string>>* pParametersPtr,
                             std::map<std::string, std::set<std::string>>* pParametersToModifyInPlacePtr,
                             bool* pTriedToModifyParametersPtr) const
{
  bool res = false;
  auto it = pOtherFacts.find(name);
  if (it != pOtherFacts.end())
    for (const auto& currOtherFact : it->second)
      if (isInOtherFact(currOtherFact, pParametersAreForTheFact, pNewParametersPtr, pParametersPtr,
                        pParametersToModifyInPlacePtr, pTriedToModifyParametersPtr))
        res = true;
  return res;
}


bool Fact::isInOtherFact(const Fact& pOtherFact,
                         bool pParametersAreForTheFact,
                         std::map<std::string, std::set<std::string>>* pNewParametersPtr,
                         const std::map<std::string, std::set<std::string>>* pParametersPtr,
                         std::map<std::string, std::set<std::string>>* pParametersToModifyInPlacePtr,
                         bool* pTriedToModifyParametersPtr,
                         bool pIgnoreValues) const
{
  if (pOtherFact.name != name ||
      pOtherFact.arguments.size() != arguments.size())
    return false;

  std::map<std::string, std::set<std::string>> newPotentialParameters;
  std::map<std::string, std::set<std::string>> newParametersInPlace;
  auto doesItMatch = [&](const std::string& pFactValue, const std::string& pValueToLookFor) {
    if (pFactValue == pValueToLookFor ||
        pFactValue == Fact::anyValue)
      return true;

    if (pParametersPtr != nullptr)
    {
      auto itParam = pParametersPtr->find(pFactValue);
      if (itParam != pParametersPtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        if (pNewParametersPtr != nullptr)
          newPotentialParameters[pFactValue].insert(pValueToLookFor);
        else if (pTriedToModifyParametersPtr != nullptr)
          *pTriedToModifyParametersPtr = true;
        return true;
      }
    }

    if (pParametersToModifyInPlacePtr != nullptr)
    {
      auto itParam = pParametersToModifyInPlacePtr->find(pFactValue);
      if (itParam != pParametersToModifyInPlacePtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        newParametersInPlace[pFactValue].insert(pValueToLookFor);
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

  std::optional<bool> resOpt;
  if (pParametersAreForTheFact)
  {
    if (pIgnoreValues || doesItMatch(value, pOtherFact.value))
      resOpt.emplace(pOtherFact.isValueNegated == isValueNegated);
  }
  else
  {
    if (pIgnoreValues || doesItMatch(pOtherFact.value, value))
      resOpt.emplace(pOtherFact.isValueNegated == isValueNegated);
  }
  if (!resOpt)
    resOpt.emplace(pOtherFact.isValueNegated != isValueNegated);

  if (*resOpt)
  {
    if (pNewParametersPtr != nullptr & !newPotentialParameters.empty())
    {
      if (pNewParametersPtr->empty())
      {
        *pNewParametersPtr = std::move(newPotentialParameters);
      }
      else
      {
        for (auto& currNewPotParam : newPotentialParameters)
          (*pNewParametersPtr)[currNewPotParam.first].insert(currNewPotParam.second.begin(), currNewPotParam.second.end());
      }
    }

    if (pParametersToModifyInPlacePtr != nullptr & !newParametersInPlace.empty())
    {
      if (pParametersToModifyInPlacePtr->empty())
      {
        *pParametersToModifyInPlacePtr = std::move(newParametersInPlace);
      }
      else
      {
        for (auto& currNewPotParam : newParametersInPlace)
          (*pParametersToModifyInPlacePtr)[currNewPotParam.first].insert(currNewPotParam.second.begin(), currNewPotParam.second.end());
      }
    }

    return true;
  }
  return false;
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
