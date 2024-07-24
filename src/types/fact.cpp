#include <contextualplanner/types/fact.hpp>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <optional>
#include <stdexcept>
#include <contextualplanner/types/factoptional.hpp>
#include <contextualplanner/types/ontology.hpp>


namespace cp
{
namespace {

void _parametersToStr(std::string& pStr,
                      const std::vector<Entity>& pParameters)
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

bool _isInside(const Entity& pEntity,
               const std::vector<std::string>* pEltsPtr)
{
  if (pEltsPtr == nullptr)
    return false;
  auto lastElt = pEltsPtr->end();
  return std::find(pEltsPtr->begin(), lastElt, pEntity.value) != lastElt;
}

bool _isInside(const Entity& pEntity,
               const std::map<std::string, std::set<Entity>>* pEltsPtr)
{
  if (pEltsPtr == nullptr)
    return false;
  auto it = pEltsPtr->find(pEntity.value);
  return it != pEltsPtr->end() && it->second.empty();
}


}

const Entity Fact::anyValue = Entity("*");
const Entity Fact::undefinedValue = Entity("undefined");
std::string Fact::punctualPrefix = "~punctual~";

Fact::Fact(const std::string& pStr,
           const Ontology& pOntology,
           const SetOfEntities& pEntities,
           const char* pSeparatorPtr,
           bool* pIsFactNegatedPtr,
           std::size_t pBeginPos,
           std::size_t* pResPos)
  : name(),
    arguments(),
    fluent(),
    isValueNegated(false),
    predicate("_not_set", pOntology.types)
{
  auto resPos = fillFactFromStr(pStr, pOntology, pEntities, pSeparatorPtr, pBeginPos, pIsFactNegatedPtr);
  if (pResPos != nullptr)
    *pResPos = resPos;
}

Fact::~Fact()
{
}

Fact::Fact(const Fact& pOther)
  : name(pOther.name),
    arguments(pOther.arguments),
    fluent(pOther.fluent),
    isValueNegated(pOther.isValueNegated),
    predicate(pOther.predicate)
{
}

Fact::Fact(Fact&& pOther) noexcept
  : name(std::move(pOther.name)),
    arguments(std::move(pOther.arguments)),
    fluent(std::move(pOther.fluent)),
    isValueNegated(std::move(pOther.isValueNegated)),
    predicate(std::move(pOther.predicate))
{
}

Fact& Fact::operator=(const Fact& pOther) {
  name = pOther.name;
  arguments = pOther.arguments;
  fluent = pOther.fluent;
  isValueNegated = pOther.isValueNegated;
  predicate = pOther.predicate;
  return *this;
}

Fact& Fact::operator=(Fact&& pOther) noexcept {
    name = std::move(pOther.name);
    arguments = std::move(pOther.arguments);
    fluent = std::move(pOther.fluent);
    isValueNegated = std::move(pOther.isValueNegated);
    predicate = std::move(pOther.predicate);
    return *this;
}


bool Fact::operator<(const Fact& pOther) const
{
  if (name != pOther.name)
    return name < pOther.name;
  if (fluent != pOther.fluent)
    return fluent < pOther.fluent;
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
      fluent == pOther.fluent && isValueNegated == pOther.isValueNegated &&
      predicate == pOther.predicate;
}


bool Fact::areEqualWithoutFluentConsideration(const Fact& pFact,
                                              const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                              const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2) const
{
  if (pFact.name != name ||
      pFact.arguments.size() != arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue &&
        !(_isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr2)))
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
      pFact.fluent != fluent)
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue &&
        *itParam != pArgToIgnore)
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return true;
}



bool Fact::areEqualExceptAnyValues(const Fact& pOther,
                                   const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                   const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2,
                                   const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr) const
{
  if (name != pOther.name || arguments.size() != pOther.arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pOther.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue &&
        !(_isInside(*itParam, pThisFactParametersToConsiderAsAnyValuePtr)) &&
        !(_isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr2)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  if (!fluent && !pOther.fluent)
    return isValueNegated == pOther.isValueNegated;
  if (fluent &&
      (*fluent == anyValue ||
       _isInside(*fluent, pThisFactParametersToConsiderAsAnyValuePtr)))
    return isValueNegated == pOther.isValueNegated;
  if (pOther.fluent &&
      (*pOther.fluent == anyValue ||
       _isInside(*pOther.fluent, pOtherFactParametersToConsiderAsAnyValuePtr) ||
       _isInside(*pOther.fluent, pOtherFactParametersToConsiderAsAnyValuePtr2)))
    return isValueNegated == pOther.isValueNegated;

  if ((fluent && !pOther.fluent) || (!fluent && pOther.fluent) || *fluent != *pOther.fluent)
    return isValueNegated != pOther.isValueNegated;

  return isValueNegated == pOther.isValueNegated;
}


bool Fact::areEqualExceptAnyValuesAndFluent(const Fact& pOther,
                                            const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                            const std::map<std::string, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2,
                                            const std::vector<std::string>* pThisFactParametersToConsiderAsAnyValuePtr) const
{
  if (name != pOther.name || arguments.size() != pOther.arguments.size())
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pOther.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam != *itOtherParam && *itParam != anyValue && *itOtherParam != anyValue &&
        !(_isInside(*itParam, pThisFactParametersToConsiderAsAnyValuePtr)) &&
        !(_isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr2)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return isValueNegated == pOther.isValueNegated;
}


bool Fact::isPunctual() const
{
  return name.compare(0, punctualPrefix.size(), punctualPrefix) == 0;
}


bool Fact::hasArgumentOrValue(
    const std::string& pArgumentOrValue) const
{
  if (fluent == pArgumentOrValue)
    return true;

  auto itParam = arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam == pArgumentOrValue)
      return true;
    ++itParam;
  }
  return false;
}


std::optional<Entity> Fact::tryToExtractArgumentFromExample(
    const std::string& pArgument,
    const Fact& pExampleFact) const
{
  if (name != pExampleFact.name ||
      isValueNegated != pExampleFact.isValueNegated ||
      arguments.size() != pExampleFact.arguments.size())
    return {};

  std::optional<Entity> res;
  if (fluent && pExampleFact.fluent && *fluent == pArgument)
    res = *pExampleFact.fluent;

  auto itParam = arguments.begin();
  auto itOtherParam = pExampleFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam == pArgument)
      res = *itOtherParam;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}

std::optional<Entity> Fact::tryToExtractArgumentFromExampleWithoutFluentConsideration(
    const std::string& pArgument,
    const Fact& pExampleFact) const
{
  if (name != pExampleFact.name ||
      isValueNegated != pExampleFact.isValueNegated ||
      arguments.size() != pExampleFact.arguments.size())
    return {};

  std::optional<Entity> res;
  auto itParam = arguments.begin();
  auto itOtherParam = pExampleFact.arguments.begin();
  while (itParam != arguments.end())
  {
    if (*itParam == pArgument)
      res = *itOtherParam;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}



bool Fact::isPatternOf(
    const std::map<std::string, std::set<Entity>>& pPossibleArguments,
    const Fact& pFactExample) const
{
  if (name != pFactExample.name ||
      isValueNegated != pFactExample.isValueNegated ||
      arguments.size() != pFactExample.arguments.size())
    return false;

  auto isOk = [&](const Entity& pPatternVal,
                  const Entity& pExempleVal) {
    auto itVal = pPossibleArguments.find(pPatternVal.value);
    if (itVal != pPossibleArguments.end())
    {
      if (!itVal->second.empty() &&
          itVal->second.count(pExempleVal) == 0)
        return false;
    }
    return true;
  };

  if (fluent && !pFactExample.fluent)
    return false;
  if (!fluent && pFactExample.fluent)
    return false;
  if (fluent && pFactExample.fluent && !isOk(*fluent, *pFactExample.fluent))
    return false;

  auto itParam = arguments.begin();
  auto itOtherParam = pFactExample.arguments.begin();
  while (itParam != arguments.end())
  {
    if (!isOk(*itParam, *itOtherParam))
      return false;
    ++itParam;
    ++itOtherParam;
  }
  return true;
}



void Fact::replaceArguments(const std::map<std::string, Entity>& pCurrentArgumentsToNewArgument)
{
  if (fluent)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(fluent->value);
    if (itValueParam != pCurrentArgumentsToNewArgument.end())
      fluent = itValueParam->second;
  }

  for (auto& currParam : arguments)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(currParam.value);
    if (itValueParam != pCurrentArgumentsToNewArgument.end())
      currParam = itValueParam->second;
  }
}

void Fact::replaceArguments(const std::map<std::string, std::set<Entity>>& pCurrentArgumentsToNewArgument)
{
  if (fluent)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(fluent->value);
    if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
      fluent = *itValueParam->second.begin();
  }

  for (auto& currParam : arguments)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(currParam.value);
    if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
      currParam = *itValueParam->second.begin();
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
  if (fluent)
  {
    if (isValueNegated)
      res += "!=" + fluent->value;
    else
      res += "=" + fluent->value;
  }
  return res;
}


Fact Fact::fromStr(const std::string& pStr,
                   const Ontology& pOntology,
                   const SetOfEntities& pEntities,
                   bool* pIsFactNegatedPtr)
{
  return Fact(pStr, pOntology, pEntities, nullptr, pIsFactNegatedPtr);
}


std::size_t Fact::fillFactFromStr(
    const std::string& pStr,
    const Ontology& pOntology,
    const SetOfEntities& pEntities,
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
        auto argumentName = cp::Fact(pStr, pOntology, pEntities, &separatorOfParameters, &isValueNegated, pos, &pos).toStr();
        if (!pOntology.empty())
        {
          auto* entityPtr = pOntology.constants.valueToEntity(argumentName);
          if (entityPtr == nullptr)
            throw std::runtime_error("\"" + argumentName + "\" is not a entity value in fact: \"" + pStr + "\"");
          arguments.emplace_back(*entityPtr);
        }
        else
        {
          arguments.emplace_back(argumentName);
        }
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
    {
      name = pStr.substr(beginPos, pos - beginPos);
    }
    else if (pos > beginPos)
    {
      auto fluentStr = pStr.substr(beginPos, pos - beginPos);

      if (!pOntology.empty())
      {
        auto* entityPtr = pOntology.constants.valueToEntity(fluentStr);
        if (entityPtr == nullptr)
          throw std::runtime_error("\"" + fluentStr + "\" is not a entity value from fact: \"" + pStr + "\"");
        fluent.emplace(*entityPtr);
      }
      else
      {
        fluent = fluentStr;
      }
    }
  }

  if (!pOntology.empty())
  {
    const auto* predicatePtr = pOntology.predicates.nameToPredicate(name);
    if (predicatePtr == nullptr)
      throw std::runtime_error("\"" + name + "\" is not a predicate name in fact: \"" + pStr + "\"");
    predicate = *predicatePtr;
    if (predicate.parameters.size() != arguments.size())
      throw std::runtime_error("\"" + pStr + "\" does not have the same number of parameters than the associated predicate \"" + predicatePtr->toStr() + "\"");
    for (auto i = 0; i < arguments.size(); ++i)
    {
      if (!arguments[i].type)
        throw std::runtime_error("\"" + arguments[i].value + "\" does not have a type, in fact \"" + pStr + "\"");
      if (!predicate.parameters[i].type)
        throw std::runtime_error("\"" + predicate.parameters[i].name + "\" does not have a type, in fact predicate \"" + predicate.toStr() + "\"");
      if (!arguments[i].type->isA(*predicate.parameters[i].type))
        throw std::runtime_error("\"" + arguments[i].toStr() + "\" is not a \"" + predicate.parameters[i].type->name + "\" in fact: \"" + pStr +
                                 "\" with predicate: \"" + predicatePtr->toStr() + "\"");
    }

    if (predicate.fluent)
    {
      if (!fluent)
        throw std::runtime_error("Fact: \"" + pStr + "\" does not have fluent but the associated predicate: \"" + predicatePtr->toStr() + "\" has a fluent");
      if (!fluent->type)
        throw std::runtime_error("\"" + fluent->toStr() + "\" does not have type in fact: \"" + pStr + "\"");
      if (!fluent->type->isA(*predicate.fluent))
        throw std::runtime_error("\"" + fluent->toStr() + "\" is not a \"" + predicate.fluent->name + "\" in fact: \"" + pStr +
                                 "\" with predicate: \"" + predicatePtr->toStr() + "\"");
    }
    else if (fluent)
    {
      throw std::runtime_error("Fact: \"" + pStr + "\" has a fluent but the associated predicate: \"" + predicatePtr->toStr() + "\" does not have a fluent");
    }
  }
  else
  {
    predicate.name = name;
    for (auto i = 0; i < arguments.size(); ++i)
      predicate.parameters.emplace_back("");
    if (fluent)
      predicate.fluent = std::make_shared<Type>("");
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
        currFactParam = anyValue;
        res = true;
      }
    }
    if (fluent == currParam)
    {
      fluent = anyValue;
      res = true;
    }
  }
  return res;
}


bool Fact::isInOtherFacts(const std::set<Fact>& pOtherFacts,
                          bool pParametersAreForTheFact,
                          std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                          const std::map<std::string, std::set<Entity>>* pParametersPtr,
                          std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr,
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
                             std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                             const std::map<std::string, std::set<Entity>>* pParametersPtr,
                             std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr,
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
                         std::map<std::string, std::set<Entity>>* pNewParametersPtr,
                         const std::map<std::string, std::set<Entity>>* pParametersPtr,
                         std::map<std::string, std::set<Entity>>* pParametersToModifyInPlacePtr,
                         bool* pTriedToModifyParametersPtr,
                         bool pIgnoreFluents) const
{
  if (pOtherFact.name != name ||
      pOtherFact.arguments.size() != arguments.size())
    return false;

  std::map<std::string, std::set<Entity>> newPotentialParameters;
  std::map<std::string, std::set<Entity>> newParametersInPlace;
  auto doesItMatch = [&](const Entity& pFactValue, const Entity& pValueToLookFor) {
    if (pFactValue == pValueToLookFor ||
        pFactValue == Fact::anyValue)
      return true;

    if (pParametersPtr != nullptr)
    {
      auto itParam = pParametersPtr->find(pFactValue.value);
      if (itParam != pParametersPtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        if (pNewParametersPtr != nullptr)
          newPotentialParameters[pFactValue.value].insert(pValueToLookFor);
        else if (pTriedToModifyParametersPtr != nullptr)
          *pTriedToModifyParametersPtr = true;
        return true;
      }
    }

    if (pParametersToModifyInPlacePtr != nullptr)
    {
      auto itParam = pParametersToModifyInPlacePtr->find(pFactValue.value);
      if (itParam != pParametersToModifyInPlacePtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        newParametersInPlace[pFactValue.value].insert(pValueToLookFor);
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
      if (*itFactParameters != *itLookForParameters &&
          ((!pParametersAreForTheFact && !doesItMatch(*itFactParameters, *itLookForParameters)) ||
           (pParametersAreForTheFact && !doesItMatch(*itLookForParameters, *itFactParameters))))
        doesParametersMatches = false;
      ++itFactParameters;
      ++itLookForParameters;
    }
    if (!doesParametersMatches)
      return false;
  }

  std::optional<bool> resOpt;
  if (pIgnoreFluents || (!fluent && !pOtherFact.fluent))
  {
    resOpt.emplace(pOtherFact.isValueNegated == isValueNegated);
  }
  else if (fluent && pOtherFact.fluent)
  {
    if (pParametersAreForTheFact)
    {
      if (doesItMatch(*fluent, *pOtherFact.fluent))
        resOpt.emplace(pOtherFact.isValueNegated == isValueNegated);
    }
    else
    {
      if (doesItMatch(*pOtherFact.fluent, *fluent))
        resOpt.emplace(pOtherFact.isValueNegated == isValueNegated);
    }
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


void Fact::replaceArgument(const std::string& pCurrent,
                           const std::string& pNew)
{
  for (auto& currParameter : arguments)
    if (currParameter == pCurrent)
      currParameter = pNew;
}


} // !cp
