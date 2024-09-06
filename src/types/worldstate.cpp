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
    if (currFact.areEqualWithoutFluentConsideration(pNegatedFact) &&
        ((currFact.isValueNegated() && currFact.fluent() == pNegatedFact.fluent()) ||
         (!currFact.isValueNegated() && currFact.fluent() != pNegatedFact.fluent())))
      return true;
  return false;
}

}


WorldState::WorldState()
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    _factsMapping(),
    _cache(std::make_unique<WorldStateCache>(*this))
{
}


WorldState::WorldState(const WorldState& pOther)
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    _factsMapping(pOther._factsMapping),
    _cache(std::make_unique<WorldStateCache>(*this, *pOther._cache))
{
}


WorldState::~WorldState()
{
}


void WorldState::operator=(const WorldState& pOther)
{
  _factsMapping = pOther._factsMapping;
  _cache = std::make_unique<WorldStateCache>(*this, *pOther._cache);
}

void WorldState::notifyActionDone(const ActionInvocationWithGoal& pOnStepOfPlannerResult,
                                  const std::unique_ptr<WorldStateModification>& pEffect,
                                  bool& pGoalChanged,
                                  GoalStack& pGoalStack,
                                  const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                                  const Ontology& pOntology,
                                  const SetOfEntities& pEntities,
                                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  if (pEffect)
  {
    if (pOnStepOfPlannerResult.actionInvocation.parameters.empty())
    {
      _modify(whatChanged, pEffect, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
    }
    else
    {
      auto effect = pEffect->cloneParamSet(pOnStepOfPlannerResult.actionInvocation.parameters);
      _modify(whatChanged, effect, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
    }
  }

  _notifyWhatChanged(whatChanged, pGoalChanged, pGoalStack, pSetOfInferences,
                     pOntology, pEntities, pNow);
}


bool WorldState::addFact(const Fact& pFact,
                         GoalStack& pGoalStack,
                         const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                         const Ontology& pOntology,
                         const SetOfEntities& pEntities,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return addFacts(std::vector<Fact>{pFact}, pGoalStack, pSetOfInferences,
                  pOntology, pEntities, pNow);
}

template<typename FACTS>
bool WorldState::addFacts(const FACTS& pFacts,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                          const Ontology& pOntology,
                          const SetOfEntities& pEntities,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _addFacts(whatChanged, pFacts, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences,
                     pOntology, pEntities, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}

template bool WorldState::addFacts<std::set<Fact>>(const std::set<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const Ontology&, const SetOfEntities&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template bool WorldState::addFacts<std::vector<Fact>>(const std::vector<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const Ontology&, const SetOfEntities&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);

bool WorldState::hasFact(const Fact& pFact) const
{
  return _factsMapping.facts().count(pFact) > 0;
}

bool WorldState::removeFact(const Fact& pFact,
                            GoalStack& pGoalStack,
                            const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                            const Ontology& pOntology,
                            const SetOfEntities& pEntities,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return removeFacts(std::vector<Fact>{pFact}, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
}

template<typename FACTS>
bool WorldState::removeFacts(const FACTS& pFacts,
                             GoalStack& pGoalStack,
                             const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                             const Ontology& pOntology,
                             const SetOfEntities& pEntities,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeFacts(whatChanged, pFacts, pGoalStack, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}


template<typename FACTS>
void WorldState::_addFacts(WhatChanged& pWhatChanged,
                           const FACTS& pFacts,
                           GoalStack& pGoalStack,
                           const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                           const Ontology& pOntology,
                           const SetOfEntities& pEntities,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.isPunctual())
    {
      pWhatChanged.punctualFacts.insert(currFact);
      continue;
    }
    if (_factsMapping.facts().count(currFact) > 0)
      continue;
    bool skipThisFact = false;

    // Remove existing facts if needed
    bool aFactWasRemoved = false;
    do
    {
      aFactWasRemoved = false;
      auto currFactMatchInWs = _factsMapping.find(currFact, true);
      for (auto itExistingFact = currFactMatchInWs.begin(); itExistingFact != currFactMatchInWs.end(); )
      {
        const auto& currExistingFact = *itExistingFact;

        if (currFact.isValueNegated() && !currExistingFact.isValueNegated() && currFact.fluent() != currExistingFact.fluent())
          skipThisFact = true;

        if (currFact.arguments() == currExistingFact.arguments() &&
            ((!currFact.isValueNegated() && !currExistingFact.isValueNegated() && currFact.fluent() != currExistingFact.fluent()) ||
             (currFact.isValueNegated() && !currExistingFact.isValueNegated() && currFact.fluent() == currExistingFact.fluent()) ||
             (!currFact.isValueNegated() && currExistingFact.isValueNegated())))
        {
          WhatChanged subWhatChanged;
          _removeFacts(subWhatChanged, std::vector<cp::Fact>{currExistingFact}, pGoalStack, pNow);
          bool goalChanged = false;
          _notifyWhatChanged(subWhatChanged, goalChanged, pGoalStack, pSetOfInferences,
                             pOntology, pEntities, pNow);
          aFactWasRemoved = true;
          break;
        }

        if (skipThisFact)
          break;
        ++itExistingFact;
      }
      if (skipThisFact)
        continue;
    }
    while (aFactWasRemoved);

    if (!skipThisFact)
    {
      pWhatChanged.addedFacts.insert(currFact);
      _factsMapping.add(currFact);
      _cache->notifyAboutANewFact(currFact);
    }
  }
  pGoalStack._refresh(*this, pNow);
}

template void WorldState::_addFacts<std::set<Fact>>(WhatChanged&, const std::set<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const Ontology&, const SetOfEntities&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template void WorldState::_addFacts<std::vector<Fact>>(WhatChanged&, const std::vector<Fact>&, GoalStack&, const std::map<SetOfInferencesId, SetOfInferences>&, const Ontology&, const SetOfEntities&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);


template<typename FACTS>
void WorldState::_removeFacts(WhatChanged& pWhatChanged,
                              const FACTS& pFacts,
                              GoalStack& pGoalStack,
                              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
    _removeAFact(pWhatChanged, currFact);
  pGoalStack._refresh(*this, pNow);
}


void WorldState::_removeAFact(WhatChanged& pWhatChanged,
                              const Fact& pFact)
{
  pWhatChanged.removedFacts.insert(pFact);
  _factsMapping.erase(pFact);
  _cache->clear();
}

void WorldState::_modify(WhatChanged& pWhatChanged,
                         const std::unique_ptr<WorldStateModification>& pWsModif,
                         GoalStack& pGoalStack,
                         const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                         const Ontology& pOntology,
                         const SetOfEntities& pEntities,
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

  _addFacts(pWhatChanged, factsToAdd, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
  _removeFacts(pWhatChanged, factsToRemove, pGoalStack, pNow);
}


bool WorldState::modify(const std::unique_ptr<WorldStateModification>& pWsModif,
                        GoalStack& pGoalStack,
                        const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                        const Ontology& pOntology,
                        const SetOfEntities& pEntities,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modify(whatChanged, pWsModif, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences,
                     pOntology, pEntities, pNow);
  return whatChanged.hasFactsToModifyInTheWorldForSure();
}


void WorldState::setFacts(const std::set<Fact>& pFacts,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                          const Ontology& pOntology,
                          const SetOfEntities& pEntities,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  _factsMapping.clear();
  for (const auto& currFact : pFacts)
    _factsMapping.add(currFact);
  _cache->clear();
  WhatChanged whatChanged;
  pGoalStack._refresh(*this, pNow);
  bool goalChanged = false;
  _notifyWhatChanged(whatChanged, goalChanged, pGoalStack, pSetOfInferences,
                     pOntology, pEntities, pNow);
}


bool WorldState::canFactOptBecomeTrue(const FactOptional& pFactOptional,
                                      const std::vector<Parameter>& pParameters) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  if (!pFactOptional.isFactNegated)
    return canFactBecomeTrue(pFactOptional.fact, pParameters);

  if (_isNegatedFactCompatibleWithFacts(pFactOptional.fact, _factsMapping.facts()))
    return true;
  if (_isNegatedFactCompatibleWithFacts(pFactOptional.fact, accessibleFacts.facts()))
    return true;

  const auto& removableFacts = _cache->removableFacts();
  if (removableFacts.facts().count(pFactOptional.fact) > 0)
    return true;

  const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
  for (const auto& currRemovableFact : removableFactsWithAnyValues)
    if (pFactOptional.fact.areEqualExceptAnyValues(currRemovableFact, nullptr, nullptr, &pParameters))
      return true;

  if (_factsMapping.facts().count(pFactOptional.fact) > 0)
    return false;
  return true;
}

bool WorldState::canFactBecomeTrue(const Fact& pFact,
                                   const std::vector<Parameter>& pParameters) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  if (!pFact.isValueNegated())
  {
    if (!_factsMapping.find(pFact).empty() ||
        !accessibleFacts.find(pFact).empty())
      return true;

    const auto& accessibleFactsWithAnyValues = _cache->accessibleFactsWithAnyValues();
    for (const auto& currAccessibleFact : accessibleFactsWithAnyValues)
      if (pFact.areEqualExceptAnyValues(currAccessibleFact, nullptr, nullptr, &pParameters))
        return true;
  }
  else
  {
    if (_isNegatedFactCompatibleWithFacts(pFact, _factsMapping.facts()))
      return true;
    if (_isNegatedFactCompatibleWithFacts(pFact, accessibleFacts.facts()))
      return true;

    const auto& removableFacts = _cache->removableFacts();
    if (removableFacts.facts().count(pFact) > 0)
      return true;

    const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
    for (const auto& currRemovableFact : removableFactsWithAnyValues)
      if (pFact.areEqualExceptAnyValues(currRemovableFact, nullptr, nullptr, &pParameters))
        return true;
  }
  return false;
}

bool WorldState::canFactNameBeModified(const std::string& pFactName) const
{
  const auto& accessibleFacts = _cache->accessibleFacts();
  for (const auto& currAccessibleFact : accessibleFacts.facts())
    if (currAccessibleFact.name() == pFactName)
      return true;

  const auto& accessibleFactsWithAnyValues = _cache->accessibleFactsWithAnyValues();
  for (const auto& currAccessibleFact : accessibleFactsWithAnyValues)
    if (currAccessibleFact.name() == pFactName)
      return true;

  const auto& removableFactsWithAnyValues = _cache->removableFactsWithAnyValues();
  for (const auto& currRemovableFact : removableFactsWithAnyValues)
    if (currRemovableFact.name() == pFactName)
      return true;
  return false;
}

std::optional<Entity> WorldState::getFactFluent(const cp::Fact& pFact) const
{
  auto factMatchingInWs = _factsMapping.find(pFact, true);
  for (const auto& currFact : factMatchingInWs)
    if (currFact.arguments() == pFact.arguments())
      return currFact.fluent();
  return {};
}


void WorldState::extractPotentialArgumentsOfAFactParameter(
    std::set<Entity>& pPotentialArgumentsOfTheParameter,
    const Fact& pFact,
    const std::string& pParameter) const
{
  auto factMatchingInWs = _factsMapping.find(pFact);
  for (const auto& currFact : factMatchingInWs)
  {
    if (currFact.arguments().size() == pFact.arguments().size())
    {
      std::set<Entity> potentialNewValues;
      bool doesItMatch = true;
      for (auto i = 0; i < pFact.arguments().size(); ++i)
      {
        if (pFact.arguments()[i] == pParameter)
        {
          potentialNewValues.insert(currFact.arguments()[i]);
          continue;
        }
        if (pFact.arguments()[i] == currFact.arguments()[i])
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


bool WorldState::isOptionalFactSatisfied(const FactOptional& pFactOptional) const
{
  const auto& facts = _factsMapping.facts();
  return (pFactOptional.isFactNegated || facts.count(pFactOptional.fact) > 0) &&
      (!pFactOptional.isFactNegated || facts.count(pFactOptional.fact) == 0);
}


bool WorldState::isOptionalFactSatisfiedInASpecificContext(const FactOptional& pFactOptional,
                                                           const std::set<Fact>& pPunctualFacts,
                                                           const std::set<Fact>& pRemovedFacts,
                                                           std::map<Parameter, std::set<Entity>>* pParametersToPossibleArgumentsPtr,
                                                           std::map<Parameter, std::set<Entity>>* pParametersToModifyInPlacePtr,
                                                           bool* pCanBecomeTruePtr) const
{
  if (pFactOptional.fact.isPunctual() && !pFactOptional.isFactNegated)
    return pPunctualFacts.count(pFactOptional.fact) != 0;

  std::map<Parameter, std::set<Entity>> newParameters;
  if (pFactOptional.isFactNegated)
  {
    bool res = pFactOptional.fact.isInOtherFacts(pRemovedFacts, true, &newParameters, pParametersToPossibleArgumentsPtr, pParametersToModifyInPlacePtr);
    if (res)
    {
      if (pParametersToPossibleArgumentsPtr != nullptr)
        applyNewParams(*pParametersToPossibleArgumentsPtr, newParameters);
      return true;
    }

    auto factMatchingInWs = _factsMapping.find(pFactOptional.fact, true);
    if (!factMatchingInWs.empty())
    {
      if (pParametersToPossibleArgumentsPtr != nullptr)
      {
        std::list<std::map<Parameter, Entity>> paramPossibilities;
        unfoldMapWithSet(paramPossibilities, *pParametersToPossibleArgumentsPtr);

        for (auto& currParamPoss : paramPossibilities)
        {
          auto factToCompare = pFactOptional.fact;
          factToCompare.replaceArguments(currParamPoss);
          if (factToCompare.fluent() == Fact::anyValue)
          {
            for (const auto& currFact : factMatchingInWs)
            {
              if (currFact.areEqualExceptAnyValues(factToCompare))
              {
                if (pFactOptional.fact.fluent() != Fact::anyValue)
                {
                  if (pFactOptional.fact.fluent() && currFact.fluent())
                    newParameters = {{pFactOptional.fact.fluent()->value, {*currFact.fluent()}}};
                  applyNewParams(*pParametersToPossibleArgumentsPtr, newParameters);
                }
                return false;
              }
            }
            return true;
          }
        }
        if (pFactOptional.fact.fluent() == Fact::anyValue)
          return false;
      }

      if (pFactOptional.fact.fluent() == Fact::anyValue)
      {
        for (const auto& currFact : factMatchingInWs)
          if (currFact.areEqualExceptAnyValues(pFactOptional.fact))
            return false;
        return true;
      }
    }

    bool triedToMidfyParameters = false;
    if (pFactOptional.fact.isInOtherFactsMap(_factsMapping, true, nullptr, pParametersToPossibleArgumentsPtr, nullptr, &triedToMidfyParameters))
    {
      if (pCanBecomeTruePtr != nullptr && triedToMidfyParameters)
        *pCanBecomeTruePtr = true;
      return false;
    }
    return true;
  }

  auto res = pFactOptional.fact.isInOtherFactsMap(_factsMapping, true, &newParameters, pParametersToPossibleArgumentsPtr);
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
 const std::map<Parameter, std::set<Entity>>& pParametersToConsiderAsAnyValue,
 const std::map<Parameter, std::set<Entity>>* pParametersToConsiderAsAnyValuePtr) const
{
  auto factMatchInWs = _factsMapping.find(pFact, true);
  for (const auto& currFact : factMatchInWs)
    if (currFact.areEqualExceptAnyValuesAndFluent(pFact, &pParametersToConsiderAsAnyValue, pParametersToConsiderAsAnyValuePtr))
      if (pValueCallback(currFact))
        break;
}


void WorldState::iterateOnMatchingFacts
(const std::function<bool (const Fact&)>& pValueCallback,
 const Fact& pFact,
 const std::map<Parameter, std::set<Entity>>& pParametersToConsiderAsAnyValue,
 const std::map<Parameter, std::set<Entity>>* pParametersToConsiderAsAnyValuePtr) const
{
  auto factMatchInWs = _factsMapping.find(pFact);
  for (const auto& currFact : factMatchInWs)
    if (currFact.areEqualExceptAnyValues(pFact, &pParametersToConsiderAsAnyValue, pParametersToConsiderAsAnyValuePtr))
      if (pValueCallback(currFact))
        break;
}



void WorldState::refreshCacheIfNeeded(const Domain& pDomain)
{
  _cache->refreshIfNeeded(pDomain, _factsMapping.facts());
}


const SetOfFact& WorldState::removableFacts() const
{
  return _cache->removableFacts();
}


bool WorldState::_tryToApplyInferences(std::set<InferenceId>& pInferencesAlreadyApplied,
                                       WhatChanged& pWhatChanged,
                                       bool& pGoalChanged,
                                       GoalStack& pGoalStack,
                                       const FactToConditions::ConstMapOfFactIterator& pInferenceIds,
                                       const std::map<InferenceId, Inference>& pInferences,
                                       const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                                       const Ontology& pOntology,
                                       const SetOfEntities& pEntities,
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

        std::map<Parameter, std::set<Entity>> parametersToValues;
        for (const auto& currParam : currInference.parameters)
          parametersToValues[currParam];
        if (!currInference.condition || currInference.condition->isTrue(*this, pWhatChanged.punctualFacts, pWhatChanged.removedFacts,
                                                                        &parametersToValues))
        {
          if (currInference.factsToModify)
          {
            if (!parametersToValues.empty())
            {
              std::list<std::map<Parameter, Entity>> parametersToValuePoss;
              unfoldMapWithSet(parametersToValuePoss, parametersToValues);
              if (!parametersToValuePoss.empty())
              {
                for (const auto& currParamsPoss : parametersToValuePoss)
                {
                  auto factsToModify = currInference.factsToModify->clone(&currParamsPoss);
                  _modify(pWhatChanged, factsToModify, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
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
                    _modify(pWhatChanged, WorldStateModification::fromStr("!" + currFactToRemove->toStr(), pOntology, pEntities, {}), // Optimize to construct WorldStateModification without passing by a string
                            pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
                }
              }
            }
            else
            {
              _modify(pWhatChanged, currInference.factsToModify, pGoalStack, pSetOfInferences, pOntology, pEntities, pNow);
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
                                    const Ontology& pOntology,
                                    const SetOfEntities& pEntities,
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
          auto it = condToReachableInferences.find(currAddedFact);
          if (_tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it, inferences,
                                    pSetOfInferences, pOntology, pEntities, pNow))
            needAnotherLoop = true;
        }
        for (auto& currAddedFact : pWhatChanged.addedFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact);
          if (_tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it, inferences,
                                    pSetOfInferences, pOntology, pEntities, pNow))
            needAnotherLoop = true;
        }
        for (auto& currRemovedFact : pWhatChanged.removedFacts)
        {
          auto it = notCondToReachableInferences.find(currRemovedFact);
          if (_tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalChanged, pGoalStack, it, inferences,
                                    pSetOfInferences, pOntology, pEntities, pNow))
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
      onFactsChanged(_factsMapping.facts());
  }
}


} // !cp
