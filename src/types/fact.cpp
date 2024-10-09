#include <contextualplanner/types/fact.hpp>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <optional>
#include <stdexcept>
#include <contextualplanner/types/factoptional.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/parameter.hpp>
#include <contextualplanner/types/setoffacts.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
namespace {

void _entitiesToStr(std::string& pStr,
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

void _entitiesToValueStr(std::string& pStr,
                         const std::vector<Entity>& pParameters)
{
  bool firstIteration = true;
  for (auto& param : pParameters)
  {
    if (firstIteration)
      firstIteration = false;
    else
      pStr += ", ";
    pStr += param.value;
  }
}

bool _isInside(const Entity& pEntity,
               const std::vector<Parameter>* pParametersPtr)
{
  if (pParametersPtr == nullptr)
    return false;
  for (auto& currParam : *pParametersPtr)
  {
    if (currParam.name == pEntity.value)
    {
      if (!pEntity.match(currParam))
        continue;
      return true;
    }
  }
  return false;
}

bool _isInside(const Entity& pEntity,
               const std::map<Parameter, std::set<Entity>>* pEltsPtr)
{
  if (pEltsPtr == nullptr)
    return false;
  auto it = pEltsPtr->find(pEntity.toParameter());
  return it != pEltsPtr->end() && it->second.empty();
}


}

const Entity Fact::undefinedValue = Entity("undefined", {});
std::string Fact::punctualPrefix = "~punctual~";

Fact::Fact(const std::string& pStr,
           const Ontology& pOntology,
           const SetOfEntities& pEntities,
           const std::vector<Parameter>& pParameters,
           const char* pSeparatorPtr,
           bool* pIsFactNegatedPtr,
           std::size_t pBeginPos,
           std::size_t* pResPos)
  : predicate("_not_set", pOntology.types),
    _name(),
    _arguments(),
    _fluent(),
    _isValueNegated(false),
    _factSignature()
{
  auto resPos = fillFactFromStr(pStr, pOntology, pEntities, pParameters, pSeparatorPtr, pBeginPos, pIsFactNegatedPtr);
  if (pResPos != nullptr)
    *pResPos = resPos;
}


Fact::Fact(const std::string& pName,
           const std::vector<std::string>& pArgumentStrs,
           const std::string& pFluentStr,
           const Ontology& pOntology,
           const SetOfEntities& pEntities,
           const std::vector<Parameter>& pParameters,
           bool pIsOkIfFluentIsMissing)
  : predicate("_not_set", pOntology.types),
    _name(pName),
    _arguments(),
    _fluent(),
    _isValueNegated(false),
    _factSignature()
{
  if (!_name.empty() && _name[_name.size() - 1] == '!')
  {
    _isValueNegated = true;
    _name = _name.substr(0, _name.size() - 1);
  }

  auto* predicatePtr = pOntology.predicates.nameToPredicatePtr(_name);
  if (predicatePtr == nullptr)
    predicatePtr = pOntology.derivedPredicates.nameToPredicatePtr(_name);
  if (predicatePtr == nullptr)
    throw std::runtime_error("\"" + pName + "\" is not a predicate name or a derived predicate name");

  predicate = *predicatePtr;
  for (auto& currParam : pArgumentStrs)
    if (!currParam.empty())
      _arguments.push_back(Entity::fromUsage(currParam, pOntology, pEntities, pParameters));
  if (!pFluentStr.empty())
    _fluent = Entity::fromUsage(pFluentStr, pOntology, pEntities, pParameters);
  else if (pIsOkIfFluentIsMissing && predicate.fluent)
    _fluent = Entity(Entity::anyEntityValue(), predicate.fluent);
  _finalizeInisilizationAndValidityChecks(pOntology, pEntities, pIsOkIfFluentIsMissing);
  _resetFactSignatureCache();
}


Fact::~Fact()
{
}

Fact::Fact(const Fact& pOther)
  : predicate(pOther.predicate),
    _name(pOther._name),
    _arguments(pOther._arguments),
    _fluent(pOther._fluent),
    _isValueNegated(pOther._isValueNegated),
    _factSignature(pOther._factSignature)
{
}

Fact::Fact(Fact&& pOther) noexcept
  : predicate(std::move(pOther.predicate)),
    _name(std::move(pOther._name)),
    _arguments(std::move(pOther._arguments)),
    _fluent(std::move(pOther._fluent)),
    _isValueNegated(std::move(pOther._isValueNegated)),
    _factSignature(std::move(pOther._factSignature))
{
}

Fact& Fact::operator=(const Fact& pOther) {
  predicate = pOther.predicate;
  _name = pOther._name;
  _arguments = pOther._arguments;
  _fluent = pOther._fluent;
  _isValueNegated = pOther._isValueNegated;
  _factSignature = pOther._factSignature;
  return *this;
}

Fact& Fact::operator=(Fact&& pOther) noexcept {
    predicate = std::move(pOther.predicate);
    _name = std::move(pOther._name);
    _arguments = std::move(pOther._arguments);
    _fluent = std::move(pOther._fluent);
    _isValueNegated = std::move(pOther._isValueNegated);
    _factSignature = std::move(pOther._factSignature);
    return *this;
}


bool Fact::operator<(const Fact& pOther) const
{
  if (_name != pOther._name)
    return _name < pOther._name;
  if (_fluent != pOther._fluent)
    return _fluent < pOther._fluent;
  if (_isValueNegated != pOther._isValueNegated)
    return _isValueNegated < pOther._isValueNegated;
  std::string paramStr;
  _entitiesToStr(paramStr, _arguments);
  std::string otherParamStr;
  _entitiesToStr(otherParamStr, pOther._arguments);
  return paramStr < otherParamStr;
}

bool Fact::operator==(const Fact& pOther) const
{
  return _name == pOther._name && _arguments == pOther._arguments &&
      _fluent == pOther._fluent && _isValueNegated == pOther._isValueNegated &&
      predicate == pOther.predicate;
}



std::ostream& operator<<(std::ostream& os, const Fact& pFact) {
    // Output the desired class members to the stream
    os << pFact.toStr();
    return os;  // Return the stream so it can be chained
}

bool Fact::areEqualWithoutFluentConsideration(const Fact& pFact,
                                              const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                              const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2) const
{
  if (pFact._name != _name ||
      pFact._arguments.size() != _arguments.size())
    return false;

  auto itParam = _arguments.begin();
  auto itOtherParam = pFact._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (*itParam != *itOtherParam && !itParam->isAnyValue() && !itOtherParam->isAnyValue() &&
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
  if (pFact._name != _name ||
      pFact._arguments.size() != _arguments.size() ||
      pFact._fluent != _fluent)
    return false;

  auto itParam = _arguments.begin();
  auto itOtherParam = pFact._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (*itParam != *itOtherParam && !itParam->isAnyValue() && !itOtherParam->isAnyValue() &&
        itParam->value != pArgToIgnore)
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return true;
}



bool Fact::areEqualExceptAnyValues(const Fact& pOther,
                                   const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                   const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2,
                                   const std::vector<Parameter>* pThisFactParametersToConsiderAsAnyValuePtr) const
{
  if (_name != pOther._name || _arguments.size() != pOther._arguments.size())
    return false;

  auto itParam = _arguments.begin();
  auto itOtherParam = pOther._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (*itParam != *itOtherParam && !itParam->isAnyValue() && !itOtherParam->isAnyValue() &&
        !(_isInside(*itParam, pThisFactParametersToConsiderAsAnyValuePtr)) &&
        !(_isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr2)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  if (!_fluent && !pOther._fluent)
    return _isValueNegated == pOther._isValueNegated;
  if (_fluent &&
      (_fluent->isAnyValue() ||
       _isInside(*_fluent, pThisFactParametersToConsiderAsAnyValuePtr)))
    return _isValueNegated == pOther._isValueNegated;
  if (pOther._fluent &&
      (pOther._fluent->isAnyValue() ||
       _isInside(*pOther._fluent, pOtherFactParametersToConsiderAsAnyValuePtr) ||
       _isInside(*pOther._fluent, pOtherFactParametersToConsiderAsAnyValuePtr2)))
    return _isValueNegated == pOther._isValueNegated;

  if ((_fluent && !pOther._fluent) || (!_fluent && pOther._fluent) || *_fluent != *pOther._fluent)
    return _isValueNegated != pOther._isValueNegated;

  return _isValueNegated == pOther._isValueNegated;
}


bool Fact::areEqualExceptAnyValuesAndFluent(const Fact& pOther,
                                            const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr,
                                            const std::map<Parameter, std::set<Entity>>* pOtherFactParametersToConsiderAsAnyValuePtr2,
                                            const std::vector<Parameter>* pThisFactParametersToConsiderAsAnyValuePtr) const
{
  if (_name != pOther._name || _arguments.size() != pOther._arguments.size())
    return false;

  auto itParam = _arguments.begin();
  auto itOtherParam = pOther._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (*itParam != *itOtherParam && !itParam->isAnyValue() && !itOtherParam->isAnyValue() &&
        !(_isInside(*itParam, pThisFactParametersToConsiderAsAnyValuePtr)) &&
        !(_isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr) ||
          _isInside(*itOtherParam, pOtherFactParametersToConsiderAsAnyValuePtr2)))
      return false;
    ++itParam;
    ++itOtherParam;
  }

  return _isValueNegated == pOther._isValueNegated;
}


bool Fact::isPunctual() const
{
  return _name.compare(0, punctualPrefix.size(), punctualPrefix) == 0;
}


bool Fact::hasParameterOrFluent(const Parameter& pParameter) const
{
  if (_fluent && _fluent->match(pParameter))
    return true;

  auto itParam = _arguments.begin();
  while (itParam != _arguments.end())
  {
    if (itParam->match(pParameter))
      return true;
    ++itParam;
  }
  return false;
}


bool Fact::hasAParameter(bool pIgnoreFluent) const
{
  for (const auto& currArg : _arguments)
    if (currArg.isAParameterToFill())
      return true;
  return !pIgnoreFluent && _fluent && _fluent->isAParameterToFill();
}


std::optional<Entity> Fact::tryToExtractArgumentFromExample(const Parameter& pParameter,
                                                            const Fact& pExampleFact) const
{
  if (_name != pExampleFact._name ||
      _isValueNegated != pExampleFact._isValueNegated ||
      _arguments.size() != pExampleFact._arguments.size())
    return {};

  std::optional<Entity> res;
  if (_fluent && pExampleFact._fluent && _fluent->match(pParameter))
    res = *pExampleFact._fluent;

  auto itParam = _arguments.begin();
  auto itOtherParam = pExampleFact._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (itParam->match(pParameter))
      res = *itOtherParam;
    ++itParam;
    ++itOtherParam;
  }
  return res;
}

std::optional<Entity> Fact::tryToExtractArgumentFromExampleWithoutFluentConsideration(
    const Parameter& pParameter,
    const Fact& pExampleFact) const
{
  if (_name != pExampleFact._name ||
      _isValueNegated != pExampleFact._isValueNegated ||
      _arguments.size() != pExampleFact._arguments.size())
    return {};

  std::optional<Entity> res;
  auto itArg = _arguments.begin();
  auto itOtherArg = pExampleFact._arguments.begin();
  while (itArg != _arguments.end())
  {
    if (itArg->match(pParameter))
      res = *itOtherArg;
    ++itArg;
    ++itOtherArg;
  }
  return res;
}



bool Fact::isPatternOf(
    const std::map<Parameter, std::set<Entity>>& pPossibleArguments,
    const Fact& pFactExample) const
{
  if (_name != pFactExample._name ||
      _isValueNegated != pFactExample._isValueNegated ||
      _arguments.size() != pFactExample._arguments.size())
    return false;

  auto isOk = [&](const Entity& pPatternVal,
                  const Entity& pExempleVal) {
    auto itVal = pPossibleArguments.find(pPatternVal.toParameter());
    if (itVal != pPossibleArguments.end())
    {
      if (!itVal->second.empty() &&
          itVal->second.count(pExempleVal) == 0)
        return false;
    }
    return true;
  };

  if (_fluent && !pFactExample._fluent)
    return false;
  if (!_fluent && pFactExample._fluent)
    return false;
  if (_fluent && pFactExample._fluent && !isOk(*_fluent, *pFactExample._fluent))
    return false;

  auto itParam = _arguments.begin();
  auto itOtherParam = pFactExample._arguments.begin();
  while (itParam != _arguments.end())
  {
    if (!isOk(*itParam, *itOtherParam))
      return false;
    ++itParam;
    ++itOtherParam;
  }
  return true;
}



void Fact::replaceArguments(const std::map<Parameter, Entity>& pCurrentArgumentsToNewArgument)
{
  if (_fluent)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(_fluent->toParameter());
    if (itValueParam != pCurrentArgumentsToNewArgument.end())
      _fluent = itValueParam->second;
  }

  for (auto& currParam : _arguments)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(currParam.toParameter());
    if (itValueParam != pCurrentArgumentsToNewArgument.end())
      currParam = itValueParam->second;
  }
  _resetFactSignatureCache();
}

void Fact::replaceArguments(const std::map<Parameter, std::set<Entity>>& pCurrentArgumentsToNewArgument)
{
  if (_fluent)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(_fluent->toParameter());
    if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
      _fluent = *itValueParam->second.begin();
  }

  for (auto& currParam : _arguments)
  {
    auto itValueParam = pCurrentArgumentsToNewArgument.find(currParam.toParameter());
    if (itValueParam != pCurrentArgumentsToNewArgument.end() && !itValueParam->second.empty())
      currParam = *itValueParam->second.begin();
  }
  _resetFactSignatureCache();
}

std::string Fact::toStr(bool pPrintAnyFluent) const
{
  std::string res = _name;
  if (!_arguments.empty())
  {
    res += "(";
    _entitiesToValueStr(res, _arguments);
    res += ")";
  }
  if (_fluent)
  {
    if (!pPrintAnyFluent && _fluent->isAnyValue())
      return res;
    if (_isValueNegated)
      res += "!=" + _fluent->value;
    else
      res += "=" + _fluent->value;
  }
  return res;
}


Fact Fact::fromStr(const std::string& pStr,
                   const Ontology& pOntology,
                   const SetOfEntities& pEntities,
                   const std::vector<Parameter>& pParameters,
                   bool* pIsFactNegatedPtr)
{
  return Fact(pStr, pOntology, pEntities, pParameters, nullptr, pIsFactNegatedPtr);
}


std::size_t Fact::fillFactFromStr(
    const std::string& pStr,
    const Ontology& pOntology,
    const SetOfEntities& pEntities,
    const std::vector<Parameter>& pParameters,
    const char* pSeparatorPtr,
    std::size_t pBeginPos,
    bool* pIsFactNegatedPtr)
{
  static const char separatorOfParameters = ',';
  std::size_t pos = pBeginPos;
  try
  {
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
          _isValueNegated = true;
          if (_name.empty())
            _name = pStr.substr(beginPos, pos - beginPos);
          ++pos;
          continue;
        }
        if (pStr[pos] == '(' || pStr[pos] == separatorOfParameters)
        {
          insideParenthesis = true;
          if (_name.empty())
            _name = pStr.substr(beginPos, pos - beginPos);
          ++pos;
          auto argumentName = cp::Fact(pStr, pOntology, pEntities, pParameters, &separatorOfParameters, &_isValueNegated, pos, &pos).toStr();
          if (!argumentName.empty())
            _arguments.push_back(Entity::fromUsage(argumentName, pOntology, pEntities, pParameters));
          beginPos = pos;
          continue;
        }
        if (pStr[pos] == ')' || pStr[pos] == '=')
        {
          insideParenthesis = false;
          if (_name.empty())
            _name = pStr.substr(beginPos, pos - beginPos);
          ++pos;
          beginPos = pos;
          continue;
        }
        ++pos;
      }
      if (_name.empty())
        _name = pStr.substr(beginPos, pos - beginPos);
      else if (pos > beginPos)
      {
        auto fluentStr = pStr.substr(beginPos, pos - beginPos);
        if (!fluentStr.empty())
          _fluent = Entity::fromUsage(fluentStr, pOntology, pEntities, pParameters);
      }
    }

    predicate = pOntology.predicates.nameToPredicate(_name);
    _finalizeInisilizationAndValidityChecks(pOntology, pEntities, false);
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(std::string(e.what()) + ". The exception was thrown while parsing fact: \"" + pStr + "\"");
  }

  _resetFactSignatureCache();
  return pos;
}


bool Fact::replaceSomeArgumentsByAny(const std::vector<Parameter>& pArgumentsToReplace)
{
  bool res = false;
  for (const auto& currParam : pArgumentsToReplace)
  {
    for (auto& currFactParam : _arguments)
    {
      if (currFactParam.value == currParam.name)
      {
        currFactParam.value = Entity::anyEntityValue();
        res = true;
      }
    }
    if (_fluent && _fluent->value == currParam.name)
    {
      _fluent->value = Entity::anyEntityValue();
      res = true;
    }
  }

  _resetFactSignatureCache();
  return res;
}


bool Fact::isInOtherFacts(const std::set<Fact>& pOtherFacts,
                          bool pParametersAreForTheFact,
                          std::map<Parameter, std::set<Entity>>* pNewParametersPtr,
                          const std::map<Parameter, std::set<Entity>>* pParametersPtr,
                          std::map<Parameter, std::set<Entity>>* pParametersToModifyInPlacePtr,
                          bool* pTriedToModifyParametersPtr) const
{
  bool res = false;
  for (const auto& currOtherFact : pOtherFacts)
    if (isInOtherFact(currOtherFact, pParametersAreForTheFact, pNewParametersPtr, pParametersPtr,
                 pParametersToModifyInPlacePtr, pTriedToModifyParametersPtr))
      res = true;
  return res;
}


bool Fact::isInOtherFactsMap(const SetOfFact& pOtherFacts,
                             bool pParametersAreForTheFact,
                             std::map<Parameter, std::set<Entity>>* pNewParametersPtr,
                             const std::map<Parameter, std::set<Entity>>* pParametersPtr,
                             std::map<Parameter, std::set<Entity>>* pParametersToModifyInPlacePtr,
                             bool* pTriedToModifyParametersPtr) const
{
  bool res = false;
  auto otherFactsThatMatched = pOtherFacts.find(*this);
  for (const auto& currOtherFact : otherFactsThatMatched)
    if (isInOtherFact(currOtherFact, pParametersAreForTheFact, pNewParametersPtr, pParametersPtr,
                      pParametersToModifyInPlacePtr, pTriedToModifyParametersPtr))
      res = true;
  return res;
}


bool Fact::isInOtherFact(const Fact& pOtherFact,
                         bool pParametersAreForTheFact,
                         std::map<Parameter, std::set<Entity>>* pNewParametersPtr,
                         const std::map<Parameter, std::set<Entity>>* pParametersPtr,
                         std::map<Parameter, std::set<Entity>>* pParametersToModifyInPlacePtr,
                         bool* pTriedToModifyParametersPtr,
                         bool pIgnoreFluents) const
{
  if (pOtherFact._name != _name ||
      pOtherFact._arguments.size() != _arguments.size())
    return false;

  std::map<Parameter, std::set<Entity>> newPotentialParameters;
  std::map<Parameter, std::set<Entity>> newParametersInPlace;
  auto doesItMatch = [&](const Entity& pFactValue, const Entity& pValueToLookFor) {
    if (pFactValue == pValueToLookFor ||
        pFactValue.isAnyValue())
      return true;

    if (pParametersPtr != nullptr)
    {
      auto itParam = pParametersPtr->find(pFactValue.toParameter());
      if (itParam != pParametersPtr->end() &&
          (!pValueToLookFor.type || !itParam->first.type || pValueToLookFor.type->isA(*itParam->first.type)))
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        if (pNewParametersPtr != nullptr)
          newPotentialParameters[pFactValue.toParameter()].insert(pValueToLookFor);
        else if (pTriedToModifyParametersPtr != nullptr)
          *pTriedToModifyParametersPtr = true;
        return true;
      }
    }

    if (pParametersToModifyInPlacePtr != nullptr)
    {
      auto itParam = pParametersToModifyInPlacePtr->find(pFactValue.toParameter());
      if (itParam != pParametersToModifyInPlacePtr->end())
      {
        if (!itParam->second.empty())
          return itParam->second.count(pValueToLookFor) > 0;
        newParametersInPlace[pFactValue.toParameter()].insert(pValueToLookFor);
        return true;
      }
    }

    return false;
  };

  {
    bool doesParametersMatches = true;
    auto itFactArguments = pOtherFact._arguments.begin();
    auto itLookForArguments = _arguments.begin();
    while (itFactArguments != pOtherFact._arguments.end())
    {
      if (*itFactArguments != *itLookForArguments &&
          ((!pParametersAreForTheFact && !doesItMatch(*itFactArguments, *itLookForArguments)) ||
           (pParametersAreForTheFact && !doesItMatch(*itLookForArguments, *itFactArguments))))
        doesParametersMatches = false;
      ++itFactArguments;
      ++itLookForArguments;
    }
    if (!doesParametersMatches)
      return false;
  }

  std::optional<bool> resOpt;
  if (pIgnoreFluents || (!_fluent && !pOtherFact._fluent))
  {
    resOpt.emplace(pOtherFact._isValueNegated == _isValueNegated);
  }
  else if (_fluent && pOtherFact._fluent)
  {
    if (pParametersAreForTheFact)
    {
      if (doesItMatch(*_fluent, *pOtherFact._fluent))
        resOpt.emplace(pOtherFact._isValueNegated == _isValueNegated);
    }
    else
    {
      if (doesItMatch(*pOtherFact._fluent, *_fluent))
        resOpt.emplace(pOtherFact._isValueNegated == _isValueNegated);
    }
  }

  if (!resOpt)
    resOpt.emplace(pOtherFact._isValueNegated != _isValueNegated);

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


void Fact::replaceArgument(const Entity& pCurrent,
                           const Entity& pNew)
{
  for (auto& currParameter : _arguments)
    if (currParameter == pCurrent)
      currParameter = pNew;
}


std::map<Parameter, Entity> Fact::extratParameterToArguments() const
{
  if (_arguments.size() == predicate.parameters.size())
  {
    std::map<Parameter, Entity> res;
    for (auto i = 0; i < _arguments.size(); ++i)
      res.emplace(predicate.parameters[i], _arguments[i]);

    if (_fluent && predicate.fluent)
    {
      res.emplace(Parameter::fromType(predicate.fluent), *_fluent);
      return res;
    }
    if (!_fluent && !predicate.fluent)
      return res;
    throw std::runtime_error("Fluent difference between fact and predicate: " + toStr());
  }
  throw std::runtime_error("No same number of arguments vs predicate parameters: " + toStr());
}


std::string Fact::factSignature() const
{
  if (_factSignature != "")
    return _factSignature;

  if (CONTEXTUALPLANNER_DEBUG_FOR_TESTS)
    throw std::runtime_error("_factSignature is not set");
  return generateFactSignature();
}

std::string Fact::generateFactSignature() const
{
  auto res = _name;
  res += "(";
  bool firstArg = true;
  for (const auto& currArg : _arguments)
  {
    if (currArg.type)
    {
      if (firstArg)
        firstArg = false;
      else
        res += ", ";
      res += currArg.type->name;
    }
  }
  res += ")";

  if (_fluent)
  {
    if (_fluent->type)
      res += "=" + _fluent->type->name;
  }
  return res;
}


void Fact::generateSignatureForAllSubTypes(std::list<std::string>& pRes) const
{
  pRes.emplace_back(factSignature());

  for (std::size_t i = 0; i < _arguments.size(); ++i)
  {
    const auto& currArg = _arguments[i];
    if (currArg.type && currArg.isAParameterToFill())
    {
      for (const auto& currSubType : currArg.type->subTypes)
      {
        auto fact = *this;
        fact.setArgumentType(i, currSubType);
        fact.generateSignatureForAllSubTypes(pRes);
      }
    }
  }
}



void Fact::setArgumentType(std::size_t pIndex, const std::shared_ptr<Type>& pType)
{
  _arguments[pIndex].type = pType;
  _resetFactSignatureCache();
}

void Fact::setFluent(const std::optional<Entity>& pFluent)
{
  _fluent = pFluent;
  _resetFactSignatureCache();
}

void Fact::setFluentValue(const std::string& pFluentStr)
{
  if (_fluent)
    _fluent->value = pFluentStr;
  else
    _fluent = Entity(pFluentStr, predicate.fluent);
  _resetFactSignatureCache();
}

bool Fact::isCompleteWithAnyValueFluent() const
{
  if (_fluent && _fluent->isAnyValue())
  {
    for (const auto& currArg : _arguments)
      if (currArg.isAParameterToFill())
        return false;
    return true;
  }
  return false;
}


void Fact::_resetFactSignatureCache()
{
  _factSignature = generateFactSignature();
}


void Fact::_finalizeInisilizationAndValidityChecks(const Ontology& pOntology,
                                                   const SetOfEntities& pEntities,
                                                   bool pIsOkIfFluentIsMissing)
{
  if (predicate.parameters.size() != _arguments.size())
    throw std::runtime_error("The fact \"" + toStr() + "\" does not have the same number of parameters than the associated predicate \"" + predicate.toStr() + "\"");
  for (auto i = 0; i < _arguments.size(); ++i)
  {
    if (!predicate.parameters[i].type)
      throw std::runtime_error("\"" + predicate.parameters[i].name + "\" does not have a type, in fact predicate \"" + predicate.toStr() + "\"");
    if (!_arguments[i].type && !_arguments[i].isAnyValue())
      throw std::runtime_error("\"" + _arguments[i].value + "\" does not have a type");
    if (_arguments[i].isAParameterToFill())
    {
      predicate.parameters[i].type = Type::getSmallerType(_arguments[i].type, predicate.parameters[i].type);
      _arguments[i].type = predicate.parameters[i].type;
      continue;
    }
    if (!_arguments[i].type->isA(*predicate.parameters[i].type))
      throw std::runtime_error("\"" + _arguments[i].toStr() + "\" is not a \"" + predicate.parameters[i].type->name + "\" for predicate: \"" + predicate.toStr() + "\"");
  }

  if (predicate.fluent)
  {
    if (_fluent)
    {
      if (!_fluent->type && !_fluent->isAnyValue())
        throw std::runtime_error("\"" + _fluent->toStr() + "\" does not have type");

      if (_fluent->isAParameterToFill())
      {
        predicate.fluent = Type::getSmallerType(_fluent->type, predicate.fluent);
        _fluent->type = predicate.fluent;
      }
      else
      {
        if (!_fluent->type->isA(*predicate.fluent))
          throw std::runtime_error("\"" + _fluent->toStr() + "\" is not a \"" + predicate.fluent->name + "\" for predicate: \"" + predicate.toStr() + "\"");
      }
    }
    else if (!pIsOkIfFluentIsMissing)
    {
      throw std::runtime_error("The fact \"" + toStr() + "\" does not have fluent but the associated predicate: \"" + predicate.toStr() + "\" has a fluent");
    }

  }
  else if (_fluent)
  {
    throw std::runtime_error("The fact \"" + toStr() + "\" has a fluent but the associated predicate: \"" + predicate.toStr() + "\" does not have a fluent");
  }
}



} // !cp
