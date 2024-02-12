#include <contextualplanner/types/worldstate.hpp>
#include <list>
#include <contextualplanner/types/goalstack.hpp>
#include <contextualplanner/types/actioninvocationwithgoal.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>
#include <contextualplanner/util/util.hpp>
#include "worldstatecache.hpp"

namespace cp
{

namespace
{

bool _isNegatedFactCompatibleWithFacts(
    const Fact& pNegatedFact,
    const std::set<Fact>& pFacts)
{
  for (const auto& currFact : pFacts)
    if (currFact.areEqualWithoutValueConsideration(pNegatedFact) &&
        ((currFact.isValueNegated && currFact.value == pNegatedFact.value) ||
         (!currFact.isValueNegated && currFact.value != pNegatedFact.value)))
      return true;
  return false;
}

}


WorldState::WorldState()
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    _facts(),
    _factNamesToFacts(),
    _cache(std::make_unique<WorldStateCache>(*this))
{
}


WorldState::WorldState(const WorldState& pOther)
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    _facts(pOther._facts),
    _factNamesToFacts(pOther._factNamesToFacts),
    _cache(std::make_unique<WorldStateCache>(*this, *pOther._cache))
{
}


WorldState::~WorldState()
{
}


void WorldState::operator=(const WorldState& pOther)
{
  _facts = pOther._facts;
  _factNamesToFacts = pOther._factNamesToFacts;
  _cache = std::make_unique<WorldStateCache>(*this, *pOther._cache);
}

void WorldState::notifyActionDone(const ActionInvocationWithGoal& pOnStepOfPlannerResult,
                                  const std::unique_ptr<WorldStateModification>& pEffect,
                                  bool& pGoalChanged,
                                  GoalStack& pGoalStack,
                                  const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  if (pEffect)
  {
    if (pOnStepOfPlannerResult.actionInvocation.parameters.empty())
    {
      _modify(whatChanged, pEffect, pGoalStack, pNow);
    }
    else
    {
      auto effect = pEffect->cloneParamSet(pOnStepOfPlannerResult.actionInvocation.parameters);
      _modify(whatChanged, effect, pGoalStack, pNow);
    }
  }

  _notifyWhatChanged(whatChanged, pGoalChanged, pGoalStack, pSetOfInferences, pNow);
}


bool WorldState::addFact(const Fact& pFact,
                         GoalStack& pGoalStack,
                         const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return addFacts(std::vector<Fact>{pFact}, pGoalStack, pSetOfInferences, pNow);
}

template<typename FACTS>
bool WorldState::addFacts(const FACTS& pFacts,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _addFacts(whatChanged, pFacts, pGoalStack, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}

template bool WorldState::addFacts<std::set<Fact>>(const std::set<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template bool WorldState::addFacts<std::vector<Fact>>(const std::vector<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);

bool WorldState::hasFact(const Fact& pFact) const
{
  return _facts.count(pFact) > 0;
}

bool WorldState::removeFact(const Fact& pFact,
                            GoalStack& pGoalStack,
                            const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return removeFacts(std::vector<Fact>{pFact}, pGoalStack, pSetOfInferences, pNow);
}

template<typename FACTS>
bool WorldState::removeFacts(const FACTS& pFacts,
                             GoalStack& pGoalStack,
                             const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeFacts(whatChanged, pFacts, pGoalStack, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}


template<typename FACTS>
void WorldState::_addFacts(WhatChanged& pWhatChanged,
                           const FACTS& pFacts,
                           GoalStack& pGoalStack,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.isPunctual())
    {
      pWhatChanged.punctualFacts.insert(currFact);
      continue;
    }
    if (_facts.count(currFact) > 0)
      continue;

    // Remove existing facts if needed
    auto itFactNameToFacts = _factNamesToFacts.find(currFact.name);
    if (itFactNameToFacts != _factNamesToFacts.end())
    {
      bool skipThisFact = false;
      for (auto itExistingFact = itFactNameToFacts->second.begin(); itExistingFact != itFactNameToFacts->second.end(); )
      {
        auto& currExistingFact = *itExistingFact;
        if (currFact.arguments == currExistingFact.arguments &&
            ((!currFact.isValueNegated && !currExistingFact.isValueNegated && currFact.value != currExistingFact.value) ||
             (currFact.isValueNegated && !currExistingFact.isValueNegated && currFact.value == currExistingFact.value) ||
             (!currFact.isValueNegated && currExistingFact.isValueNegated)))
        {
          ++itExistingFact;
          _removeFacts(pWhatChanged, std::vector<cp::Fact>{currExistingFact}, pGoalStack, pNow);
          continue;
        }

        if (currFact.isValueNegated && !currExistingFact.isValueNegated && currFact.value != currExistingFact.value)
        {
          skipThisFact = true;
          break;
        }
        ++itExistingFact;
      }
      if (skipThisFact)
        continue;
    }

    pWhatChanged.addedFacts.insert(currFact);
    _facts.insert(currFact);
    _factNamesToFacts[currFact.name].insert(currFact);
    _cache->notifyAboutANewFact(currFact);
  }
  pGoalStack._refresh(*this, pNow);
}

template void WorldState::_addFacts<std::set<Fact>>(WhatChanged&, const std::set<Fact>&, GoalStack&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template void WorldState::_addFacts<std::vector<Fact>>(WhatChanged&, const std::vector<Fact>&, GoalStack&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);


template<typename FACTS>
void WorldState::_removeFacts(WhatChanged& pWhatChanged,
                              const FACTS& pFacts,
                              GoalStack& pGoalStack,
                              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
  {
    bool factRemoved = false;
    auto it = _facts.find(currFact);
    if (it == _facts.end())
    {
      // If the fact is not found, we try to find the fact with another value
      if (currFact.value == Fact::anyValue)
      {
        auto itFromFactName = _factNamesToFacts.find(currFact.name);
        if (itFromFactName != _factNamesToFacts.end())
        {
          for (const auto& currFactFromName : itFromFactName->second)
          {
            if (currFactFromName.areEqualExceptAnyValues(currFact))
            {
              it = _facts.find(currFactFromName);
              if (it != _facts.end())
              {
                _facts.erase(it);
                itFromFactName->second.erase(currFactFromName);
                if (itFromFactName->second.empty())
                  _factNamesToFacts.erase(itFromFactName);
                factRemoved = true;
                break;
              }
            }
          }
        }
      }
      if (it == _facts.end())
        continue;
    }
    pWhatChanged.removedFacts.insert(currFact);
    if (!factRemoved)
    {
      _facts.erase(it);
      auto itFactName = _factNamesToFacts.find(currFact.name);
      if (itFactName == _factNamesToFacts.end())
        assert(false);
      itFactName->second.erase(currFact);
      if (itFactName->second.empty())
        _factNamesToFacts.erase(itFactName);
    }
    _cache->clear();
  }
  pGoalStack._refresh(*this, pNow);
}


void WorldState::_modify(WhatChanged& pWhatChanged,
                         const std::unique_ptr<WorldStateModification>& pWsModif,
                         GoalStack& pGoalStack,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (!pWsModif)
    return;

  std::list<Fact> factsToAdd;
  std::list<Fact> factsToRemove;
  pWsModif->forAll(
        [&](const FactOptional& pFactOptional)
  {
    if (pFactOptional.isFactNegated)
      factsToRemove.emplace_back(pFactOptional.fact);
    else
      factsToAdd.emplace_back(pFactOptional.fact);
  }, *this);

  _addFacts(pWhatChanged, factsToAdd, pGoalStack, pNow);
  _removeFacts(pWhatChanged, factsToRemove, pGoalStack, pNow);
}


bool WorldState::modify(const std::unique_ptr<WorldStateModification>& pWsModif,
                        GoalStack& pGoalStack,
                        const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modify(whatChanged, pWsModif, pGoalStack, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}


void WorldState::setFacts(const std::set<Fact>& pFacts,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _factNamesToFacts.clear();
    for (const auto& currFact : pFacts)
      _factNamesToFacts[currFact.name].insert(currFact);
    _cache->clear();
    WhatChanged whatChanged;
    pGoalStack._refresh(*this, pNow);
    bool goalChanged = false;
    _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences, pNow);
  }
}


bool WorldState::canFactOptBecomeTrue(const FactOptional& pFactOptional) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  if (!pFactOptional.isFactNegated)
    return canFactBecomeTrue(pFactOptional.fact);

  if (_isNegatedFactCompatibleWithFacts(pFactOptional.fact, _facts))
    return true;
  if (_isNegatedFactCompatibleWithFacts(pFactOptional.fact, accessibleFacts))
    return true;

  const auto& removableFacts = _cache->removableFacts();
  if (removableFacts.count(pFactOptional.fact) > 0)
    return true;

  const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
  for (const auto& currRemovableFact : removableFactsWithAnyValues)
    if (pFactOptional.fact.areEqualExceptAnyValues(currRemovableFact))
      return true;

  if (_facts.count(pFactOptional.fact) > 0)
    return false;
  return true;
}

bool WorldState::canFactBecomeTrue(const Fact& pFact) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  if (!pFact.isValueNegated)
  {
    if (_factNamesToFacts.count(pFact.name) ||
        accessibleFacts.count(pFact) > 0)
      return true;

    const auto& accessibleFactsWithAnyValues = _cache->accessibleFactsWithAnyValues();
    for (const auto& currAccessibleFact : accessibleFactsWithAnyValues)
      if (pFact.areEqualExceptAnyValues(currAccessibleFact))
        return true;
  }
  else
  {
    if (_isNegatedFactCompatibleWithFacts(pFact, _facts))
      return true;
    if (_isNegatedFactCompatibleWithFacts(pFact, accessibleFacts))
      return true;

    const auto& removableFacts = _cache->removableFacts();
    if (removableFacts.count(pFact) > 0)
      return true;

    const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
    for (const auto& currRemovableFact : removableFactsWithAnyValues)
      if (pFact.areEqualExceptAnyValues(currRemovableFact))
        return true;
  }
  return false;
}

bool WorldState::canFactNameBeModified(const std::string& pFactName) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  for (const auto& currAccessibleFact : accessibleFacts)
    if (currAccessibleFact.name == pFactName)
      return true;

  const auto& accessibleFactsWithAnyValues = _cache->accessibleFactsWithAnyValues();
  for (const auto& currAccessibleFact : accessibleFactsWithAnyValues)
    if (currAccessibleFact.name == pFactName)
      return true;

  const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
  for (const auto& currRemovableFact : removableFactsWithAnyValues)
    if (currRemovableFact.name == pFactName)
      return true;
  return false;
}

std::string WorldState::getFactValue(const cp::Fact& pFact) const
{
  auto itFact = _factNamesToFacts.find(pFact.name);
  if (itFact != _factNamesToFacts.end())
  {
    for (auto& currFact : itFact->second)
      if (currFact.arguments == pFact.arguments)
        return currFact.value;
  }
  return "";
}


void WorldState::extractPotentialArgumentsOfAFactParameter(
    std::set<Fact>& pPotentialArgumentsOfTheParameter,
    const Fact& pFact,
    const std::string& pParameter) const
{
  auto itFact = _factNamesToFacts.find(pFact.name);
  if (itFact != _factNamesToFacts.end())
  {
    for (auto& currFact : itFact->second)
    {
      if (currFact.arguments.size() == pFact.arguments.size())
      {
        std::set<Fact> potentialNewValues;
        bool doesItMatch = true;
        for (auto i = 0; i < pFact.arguments.size(); ++i)
        {
          if (pFact.arguments[i] == pParameter)
          {
            potentialNewValues.insert(currFact.arguments[i].fact);
            continue;
          }
          if (pFact.arguments[i] == currFact.arguments[i])
            continue;
          doesItMatch = false;
          break;
        }
        if (doesItMatch)
        {
          if (pPotentialArgumentsOfTheParameter.empty())
            pPotentialArgumentsOfTheParameter = std::move(potentialNewValues);
          else
            pPotentialArgumentsOfTheParameter.insert(potentialNewValues.begin(), potentialNewValues.end());
        }
      }
    }
  }
}


bool WorldState::isOptionalFactSatisfied(const FactOptional& pFactOptional) const
{
  return (pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) > 0) &&
      (!pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) == 0);
}


bool WorldState::isOptionalFactSatisfiedInASpecificContext(
    const FactOptional& pFactOptional,
    const std::set<Fact>& pPunctualFacts,
    const std::set<Fact>& pRemovedFacts,
    std::map<std::string, std::set<std::string>>* pParametersToPossibleArgumentsPtr,
    std::map<std::string, std::set<std::string>>* pParametersToModifyInPlacePtr,
    bool* pCanBecomeTruePtr) const
{
  if (pFactOptional.fact.isPunctual() && !pFactOptional.isFactNegated)
    return pPunctualFacts.count(pFactOptional.fact) != 0;

  std::map<std::string, std::set<std::string>> newParameters;
  if (pFactOptional.isFactNegated)
  {
    bool res = pFactOptional.fact.isInOtherFacts(pRemovedFacts, true, &newParameters, pParametersToPossibleArgumentsPtr, pParametersToModifyInPlacePtr);
    if (res)
    {
      if (pParametersToPossibleArgumentsPtr != nullptr)
        applyNewParams(*pParametersToPossibleArgumentsPtr, newParameters);
      return true;
    }

    auto itFacts = _factNamesToFacts.find(pFactOptional.fact.name);
    if (itFacts != _factNamesToFacts.end())
    {
      if (pParametersToPossibleArgumentsPtr != nullptr)
      {
        std::list<std::map<std::string, std::string>> paramPossibilities;
        unfoldMapWithSet(paramPossibilities, *pParametersToPossibleArgumentsPtr);

        for (auto& currParamPoss : paramPossibilities)
        {
          auto factToCompare = pFactOptional.fact;
          factToCompare.replaceArguments(currParamPoss);
          if (factToCompare.value == Fact::anyValue)
          {
            for (auto& currFact : itFacts->second)
            {
              if (currFact.areEqualExceptAnyValues(factToCompare))
              {
                if (pFactOptional.fact.value != Fact::anyValue)
                {
                  std::map<std::string, std::set<std::string>> newParameters =
                  {{pFactOptional.fact.value, {currFact.value}}};
                  applyNewParams(*pParametersToPossibleArgumentsPtr, newParameters);
                }
                return false;
              }
            }
            return true;
          }
        }
        if (pFactOptional.fact.value == Fact::anyValue)
          return false;
      }

      if (pFactOptional.fact.value == Fact::anyValue)
      {
        for (auto& currFact : itFacts->second)
          if (currFact.areEqualExceptAnyValues(pFactOptional.fact))
            return false;
        return true;
      }
    }

    bool triedToMidfyParameters = false;
    if (pFactOptional.fact.isInOtherFactsMap(_factNamesToFacts, true, nullptr, pParametersToPossibleArgumentsPtr, nullptr, &triedToMidfyParameters))
    {
      if (pCanBecomeTruePtr != nullptr && triedToMidfyParameters)
        *pCanBecomeTruePtr = true;
      return false;
    }
    return true;
  }

  auto res = pFactOptional.fact.isInOtherFactsMap(_factNamesToFacts, true, &newParameters, pParametersToPossibleArgumentsPtr);
  if (pParametersToPossibleArgumentsPtr != nullptr)
    applyNewParams(*pParametersToPossibleArgumentsPtr, newParameters);
  return res;
}



bool WorldState::isGoalSatisfied(const Goal& pGoal) const
{
  auto* condFactOptPtr = pGoal.conditionFactOptionalPtr();
  return (condFactOptPtr != nullptr && !isOptionalFactSatisfied(*condFactOptPtr)) ||
      pGoal.objective().isTrue(*this);
}


void WorldState::iterateOnMatchingFactsWithoutFluentConsideration
(const std::function<bool (const Fact&)>& pValueCallback,
 const Fact& pFact,
 const std::map<std::string, std::set<std::string>>& pParametersToConsiderAsAnyValue,
 const std::map<std::string, std::set<std::string>>* pParametersToConsiderAsAnyValuePtr) const
{
  auto it = _factNamesToFacts.find(pFact.name);
  if (it != _factNamesToFacts.end())
    for (const Fact& currFact : it->second)
      if (currFact.areEqualExceptAnyValuesAndFluent(pFact, &pParametersToConsiderAsAnyValue, pParametersToConsiderAsAnyValuePtr))
        if (pValueCallback(currFact))
          break;
}


void WorldState::iterateOnMatchingFacts
(const std::function<bool (const Fact&)>& pValueCallback,
 const Fact& pFact,
 const std::map<std::string, std::set<std::string>>& pParametersToConsiderAsAnyValue,
 const std::map<std::string, std::set<std::string>>* pParametersToConsiderAsAnyValuePtr) const
{
  auto it = _factNamesToFacts.find(pFact.name);
  if (it != _factNamesToFacts.end())
    for (const Fact& currFact : it->second)
      if (currFact.areEqualExceptAnyValues(pFact, &pParametersToConsiderAsAnyValue, pParametersToConsiderAsAnyValuePtr))
        if (pValueCallback(currFact))
          break;
}



void WorldState::refreshCacheIfNeeded(const Domain& pDomain)
{
  _cache->refreshIfNeeded(pDomain, _facts);
}


const std::set<Fact>& WorldState::removableFacts() const
{
  return _cache->removableFacts();
}


bool WorldState::_tryToApplyInferences(std::set<InferenceId>& pInferencesAlreadyApplied,
                                       WhatChanged& pWhatChanged,
                                       bool& pGoalChanged,
                                       GoalStack& pGoalStack,
                                       const std::set<InferenceId>& pInferenceIds,
                                       const std::map<InferenceId, Inference>& pInferences,
                                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  bool somethingChanged = false;
  for (const auto& currInferenceId : pInferenceIds)
  {
    if (pInferencesAlreadyApplied.count(currInferenceId) == 0)
    {
      pInferencesAlreadyApplied.insert(currInferenceId);
      auto itInference = pInferences.find(currInferenceId);
      if (itInference != pInferences.end())
      {
        const Inference& currInference = itInference->second;

        std::map<std::string, std::set<std::string>> parametersToValues;
        for (const auto& currParam : currInference.parameters)
          parametersToValues[currParam];
        if (!currInference.condition || currInference.condition->isTrue(*this, pWhatChanged.punctualFacts, pWhatChanged.removedFacts,
                                                                        &parametersToValues))
        {
          if (currInference.factsToModify)
          {
            if (!parametersToValues.empty())
            {
              std::list<std::map<std::string, std::string>> parametersToValuePoss;
              unfoldMapWithSet(parametersToValuePoss, parametersToValues);
              if (!parametersToValuePoss.empty())
              {
                for (const auto& currParamsPoss : parametersToValuePoss)
                {
                  auto factsToModify = currInference.factsToModify->clone(&currParamsPoss);
                  _modify(pWhatChanged, factsToModify, pGoalStack, pNow);
                }
              }
              else
              {
                const auto* optFactPtr = currInference.factsToModify->getOptionalFact();
                // If there is no parameter possible value and if the effect is to remove a fact then we remove of the matching facts
                if (optFactPtr != nullptr && optFactPtr->isFactNegated)
                {
                  std::list<const Fact*> factsToRemove;
                  iterateOnMatchingFacts([&](const Fact& pMatchedFact) {
                    factsToRemove.emplace_back(&pMatchedFact);
                    return false;
                  }, optFactPtr->fact, parametersToValues);
                  for (auto& currFactToRemove : factsToRemove)
                    _modify(pWhatChanged, WorldStateModification::fromStr("!" + currFactToRemove->toStr()), // Optimize to construct WorldStateModification without passing by a string
                            pGoalStack, pNow);
                }
              }
            }
            else
            {
              _modify(pWhatChanged, currInference.factsToModify, pGoalStack, pNow);
            }
          }
          if (pGoalStack.addGoals(currInference.goalsToAdd, *this, pNow))
            pGoalChanged = true;
          somethingChanged = true;
        }
      }
    }
  }
  return somethingChanged;
}


void WorldState::_notifyWhatChanged(WhatChanged& pWhatChanged,
                                    bool& pGoalChanged,
                                    GoalStack& pGoalStack,
                                    const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (pWhatChanged.somethingChanged())
  {
    // manage the inferences
    std::map<SetOfInferencesId, std::set<InferenceId>> soiToInferencesAlreadyApplied;
    bool needAnotherLoop = true;
    while (needAnotherLoop)
    {
      needAnotherLoop = false;
      for (auto& currSetOfInferences : pSetOfInferences)
      {
        auto& inferences = currSetOfInferences.second.inferences();
        auto& condToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().conditionToInferences;
        auto& notCondToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().notConditionToInferences;
        auto& inferencesAlreadyApplied = soiToInferencesAlreadyApplied[currSetOfInferences.first];

        for (auto& currAddedFact : pWhatChanged.punctualFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact.name);
          if (it != condToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currAddedFact : pWhatChanged.addedFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact.name);
          if (it != condToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currRemovedFact : pWhatChanged.removedFacts)
        {
          auto it = notCondToReachableInferences.find(currRemovedFact.name);
          if (it != notCondToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
      }
    }

    if (!pWhatChanged.punctualFacts.empty())
      onPunctualFacts(pWhatChanged.punctualFacts);
    if (!pWhatChanged.addedFacts.empty())
      onFactsAdded(pWhatChanged.addedFacts);
    if (!pWhatChanged.removedFacts.empty())
      onFactsRemoved(pWhatChanged.removedFacts);
    if (pWhatChanged.hasFactsToModifyInTheWorldForSure())
      onFactsChanged(_facts);
  }
}


} // !cp
