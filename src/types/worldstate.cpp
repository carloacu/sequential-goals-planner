#include <contextualplanner/types/worldstate.hpp>
#include <map>
#include <sstream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/goalstack.hpp>
#include <contextualplanner/types/onestepofplannerresult.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{

namespace
{

void _incrementStr(std::string& pStr)
{
  if (pStr.empty())
  {
    pStr = "1";
  }
  else
  {
    try
    {
      std::stringstream ss;
      ss << lexical_cast<int>(pStr) + 1;
      pStr = ss.str();
    }
    catch (...) {}
  }
}

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



WorldState::WorldState(const WorldState& pOther)
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    _facts(pOther._facts),
    _factNamesToFacts(pOther._factNamesToFacts),
    _accessibleFacts(pOther._accessibleFacts),
    _accessibleFactsWithAnyValues(pOther._accessibleFactsWithAnyValues),
    _removableFacts(pOther._removableFacts),
    _needToAddAccessibleFacts(pOther._needToAddAccessibleFacts)
{
}


void WorldState::notifyActionDone(const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                                  const std::unique_ptr<FactModification>& pEffect,
                                  GoalStack& pGoalStack,
                                  const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  if (pEffect)
  {
    if (pOnStepOfPlannerResult.actionInstance.parameters.empty())
    {
      _modifyFacts(whatChanged, pEffect, pGoalStack, pNow);
    }
    else
    {
      auto effect = pEffect->cloneParamSet(pOnStepOfPlannerResult.actionInstance.parameters);
      _modifyFacts(whatChanged, effect, pGoalStack, pNow);
    }
  }

  _notifyWhatChanged(whatChanged, pGoalStack, pSetOfInferences, pNow);
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
  _notifyWhatChanged(whatChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsModifications();
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
  _notifyWhatChanged(whatChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsModifications();
}


template<typename FACTS>
void WorldState::_addFacts(WhatChanged& pWhatChanged,
                           const FACTS& pFacts,
                           GoalStack& pGoalStack,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.isUnreachable())
      continue;
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

    auto itAccessible = _accessibleFacts.find(currFact);
    if (itAccessible != _accessibleFacts.end())
      _accessibleFacts.erase(itAccessible);
    else
      clearAccessibleAndRemovableFacts();
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
    auto it = _facts.find(currFact);
    if (it == _facts.end())
    {
      // If the fact is not found, we try to find the fact with another value
      if (currFact.value == Fact::anyValue)
      {
        auto itFromFactName = _factNamesToFacts.find(currFact.name);
        if (itFromFactName != _factNamesToFacts.end())
        {
          for (auto& currFactFromName : itFromFactName->second)
          {
            if (currFactFromName.areEqualExceptAnyValues(currFact))
            {
              it = _facts.find(currFactFromName);
              if (it != _facts.end())
                break;
            }
          }
        }
      }
      if (it == _facts.end())
        continue;
    }
    pWhatChanged.removedFacts.insert(currFact);
    _facts.erase(it);
    {
      auto itFactName = _factNamesToFacts.find(currFact.name);
      if (itFactName == _factNamesToFacts.end())
        assert(false);
      itFactName->second.erase(currFact);
      if (itFactName->second.empty())
        _factNamesToFacts.erase(itFactName);
    }
    clearAccessibleAndRemovableFacts();
  }
  pGoalStack._refresh(*this, pNow);
}


void WorldState::clearAccessibleAndRemovableFacts()
{
  _needToAddAccessibleFacts = true;
  _accessibleFacts.clear();
  _accessibleFactsWithAnyValues.clear();
  _removableFacts.clear();
}

bool WorldState::modifyFacts(const std::unique_ptr<FactModification>& pFactModification,
                             GoalStack& pGoalStack,
                             const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modifyFacts(whatChanged, pFactModification, pGoalStack, pNow);
  _notifyWhatChanged(whatChanged, pGoalStack, pSetOfInferences, pNow);
  return whatChanged.hasFactsModifications();
}


void WorldState::_modifyFacts(WhatChanged& pWhatChanged,
                              const std::unique_ptr<FactModification>& pFactModification,
                              GoalStack& pGoalStack,
                              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (!pFactModification)
    return;

  std::list<Fact> factsToAdd;
  std::list<Fact> factsToRemove;
  pFactModification->forAll(
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
    clearAccessibleAndRemovableFacts();
    WhatChanged whatChanged;
    pGoalStack._refresh(*this, pNow);
    _notifyWhatChanged(whatChanged, pGoalStack, pSetOfInferences, pNow);
  }
}


bool WorldState::canFactBecomeTrue(const Fact& pFact) const
{
  if (!pFact.isValueNegated)
  {
    if (_facts.count(pFact) > 0 ||
        _accessibleFacts.count(pFact) > 0)
      return true;

    for (const auto& currAccessibleFact : _accessibleFactsWithAnyValues)
      if (pFact.areEqualExceptAnyValues(currAccessibleFact))
        return true;
  }
  else
  {
    if (_isNegatedFactCompatibleWithFacts(pFact, _facts))
      return true;
    if (_isNegatedFactCompatibleWithFacts(pFact, _accessibleFacts))
      return true;
  }
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


void WorldState::forAllInstruction(const std::string& pParameterName,
                                const Fact& pFact,
                                std::set<Fact>& pParameterValues) const
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
          if (pFact.arguments[i] == pParameterName)
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
          pParameterValues.insert(potentialNewValues.begin(), potentialNewValues.end());
      }
    }
  }
}


void WorldState::fillAccessibleFacts(const Domain& pDomain)
{
  if (!_needToAddAccessibleFacts)
    return;
  auto& setOfInferences = pDomain.getSetOfInferences();
  FactsAlreadyChecked factsAlreadychecked;
  for (const auto& currFact : _facts)
  {
    if (_accessibleFacts.count(currFact) == 0)
    {
      auto itPrecToActions = pDomain.preconditionToActions().find(currFact.name);
      if (itPrecToActions != pDomain.preconditionToActions().end())
        _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain,
                                             factsAlreadychecked, setOfInferences);
    }
  }
  _feedAccessibleFactsFromSetOfActions(pDomain.actionsWithoutFactToAddInPrecondition(), pDomain,
                                       factsAlreadychecked, setOfInferences);
  _needToAddAccessibleFacts = false;
}


void WorldState::_feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                                      const Domain& pDomain,
                                                      FactsAlreadyChecked& pFactsAlreadychecked,
                                                      const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences)
{
  auto& actions = pDomain.actions();
  for (const auto& currAction : pActions)
  {
    auto itAction = actions.find(currAction);
    if (itAction != actions.end())
    {
      auto& action = itAction->second;
      _feedAccessibleFactsFromDeduction(action.precondition, action.effect, action.parameters,
                                        pDomain, pFactsAlreadychecked, pSetOfInferences);
    }
  }
}


void WorldState::_feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                                         const std::map<InferenceId, Inference>& pAllInferences,
                                                         const Domain& pDomain,
                                                         FactsAlreadyChecked& pFactsAlreadychecked,
                                                         const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences)
{
  for (const auto& currInference : pInferences)
  {
    auto itInference = pAllInferences.find(currInference);
    if (itInference != pAllInferences.end())
    {
      auto& inference = itInference->second;
      std::vector<std::string> parameters;
      _feedAccessibleFactsFromDeduction(inference.condition, inference.factsToModify->clone(nullptr),
                                        parameters, pDomain, pFactsAlreadychecked, pSetOfInferences);
    }
  }
}

void WorldState::_feedAccessibleFactsFromDeduction(const std::unique_ptr<Condition>& pCondition,
                                                const ProblemModification& pEffect,
                                                const std::vector<std::string>& pParameters,
                                                const Domain& pDomain,
                                                FactsAlreadyChecked& pFactsAlreadychecked,
                                                const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences)
{
  if (!pCondition || pCondition->canBecomeTrue(*this))
  {
    std::set<Fact> accessibleFactsToAdd;
    std::vector<Fact> accessibleFactsToAddWithAnyValues;
    std::set<Fact> removableFactsToAdd;

    pEffect.forAll([&](const cp::FactOptional& pFactOpt) {
      if (!pFactOpt.isFactNegated)
      {
        if (_facts.count(pFactOpt.fact) == 0 &&
            _accessibleFacts.count(pFactOpt.fact) == 0)
        {
          auto factToInsert = pFactOpt.fact;
          if (factToInsert.replaceSomeArgumentsByAny(pParameters))
            accessibleFactsToAddWithAnyValues.push_back(std::move(factToInsert));
          else
            accessibleFactsToAdd.insert(std::move(factToInsert));
        }
      }
      else
      {
        if (_facts.count(pFactOpt.fact) > 0 &&
            _removableFacts.count(pFactOpt.fact) == 0)
          removableFactsToAdd.insert(pFactOpt.fact);
      }
    }, *this);

    if (!accessibleFactsToAdd.empty() || !accessibleFactsToAddWithAnyValues.empty() || !removableFactsToAdd.empty())
    {
      _accessibleFacts.insert(accessibleFactsToAdd.begin(), accessibleFactsToAdd.end());
      _accessibleFactsWithAnyValues.insert(accessibleFactsToAddWithAnyValues.begin(), accessibleFactsToAddWithAnyValues.end());
      _removableFacts.insert(removableFactsToAdd.begin(), removableFactsToAdd.end());
      for (const auto& currNewFact : accessibleFactsToAdd)
        _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked, pSetOfInferences);
      for (const auto& currNewFact : accessibleFactsToAddWithAnyValues)
        _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked, pSetOfInferences);
      for (const auto& currNewFact : removableFactsToAdd)
        _feedAccessibleFactsFromNotFact(currNewFact, pDomain, pFactsAlreadychecked, pSetOfInferences);
    }
  }
}


void WorldState::_feedAccessibleFactsFromFact(const Fact& pFact,
                                              const Domain& pDomain,
                                              FactsAlreadyChecked& pFactsAlreadychecked,
                                              const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences)
{
  if (!pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain, pFactsAlreadychecked, pSetOfInferences);

  for (auto& currSetOfInferences : pSetOfInferences)
  {
    auto& allInferences = currSetOfInferences.second.inferences();
    auto& conditionToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().conditionToInferences;
    auto itCondToReachableInferences = conditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != conditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, allInferences, pDomain, pFactsAlreadychecked, pSetOfInferences);
    auto& conditionToUnreachableInferences = currSetOfInferences.second.unreachableInferenceLinks().conditionToInferences;
    auto itCondToUnreachableInferences = conditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != conditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, allInferences, pDomain, pFactsAlreadychecked, pSetOfInferences);
  }
}


void WorldState::_feedAccessibleFactsFromNotFact(const Fact& pFact,
                                                 const Domain& pDomain,
                                                 FactsAlreadyChecked& pFactsAlreadychecked,
                                                 const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences)
{
  if (!pFactsAlreadychecked.factsToRemove.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.notPreconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.notPreconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain, pFactsAlreadychecked, pSetOfInferences);

  for (auto& currSetOfInferences : pSetOfInferences)
  {
    auto& allInferences = currSetOfInferences.second.inferences();
    auto& notConditionToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().notConditionToInferences;
    auto itCondToReachableInferences = notConditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != notConditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, allInferences, pDomain, pFactsAlreadychecked, pSetOfInferences);
    auto& notConditionToUnreachableInferences = currSetOfInferences.second.unreachableInferenceLinks().notConditionToInferences;
    auto itCondToUnreachableInferences = notConditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != notConditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, allInferences, pDomain, pFactsAlreadychecked, pSetOfInferences);
  }
}


bool WorldState::isOptionalFactSatisfied(const FactOptional& pFactOptional) const
{
  return (pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) > 0) &&
      (!pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) == 0);
}


bool WorldState::isGoalSatisfied(const Goal& pGoal) const
{
  auto* condFactOptPtr = pGoal.conditionFactOptionalPtr();
  return (condFactOptPtr != nullptr && !isOptionalFactSatisfied(*condFactOptPtr)) ||
      pGoal.objective().isTrue(*this);
}

bool WorldState::isFactPatternSatisfied(const FactOptional& pFactOptional,
                                        const std::set<Fact>& pPunctualFacts,
                                        const std::set<Fact>& pRemovedFacts,
                                        std::map<std::string, std::set<std::string>>* pParametersPtr,
                                        bool* pCanBecomeTruePtr) const
{
  if (pFactOptional.fact.isPunctual() && !pFactOptional.isFactNegated)
    return pPunctualFacts.count(pFactOptional.fact) != 0;

  std::map<std::string, std::set<std::string>> newParameters;
  if (pFactOptional.isFactNegated)
  {
    bool res = pFactOptional.fact.isInOtherFacts(pRemovedFacts, true, &newParameters, pParametersPtr);
    if (res)
    {
      if (pParametersPtr != nullptr)
        applyNewParams(*pParametersPtr, newParameters);
      return true;
    }

    auto itFacts = _factNamesToFacts.find(pFactOptional.fact.name);
    if (itFacts != _factNamesToFacts.end())
    {
      if (pParametersPtr != nullptr)
      {
        std::list<std::map<std::string, std::string>> paramPossibilities;
        unfoldMapWithSet(paramPossibilities, (*pParametersPtr));

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
                  applyNewParams(*pParametersPtr, newParameters);
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
    if (pFactOptional.fact.isInOtherFacts(_facts, true, nullptr, pParametersPtr, &triedToMidfyParameters))
    {
      if (pCanBecomeTruePtr != nullptr && triedToMidfyParameters)
        *pCanBecomeTruePtr = true;
      return false;
    }
    return true;
  }

  auto res = pFactOptional.fact.isInOtherFacts(_facts, true, &newParameters, pParametersPtr);
  if (pParametersPtr != nullptr)
    applyNewParams(*pParametersPtr, newParameters);
  return res;
}




bool WorldState::_tryToApplyInferences(std::set<InferenceId>& pInferencesAlreadyApplied,
                                       WhatChanged& pWhatChanged,
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
        auto& currInference = itInference->second;

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
              for (const auto& currParamsPoss : parametersToValuePoss)
              {
                auto factsToModify = currInference.factsToModify->clone(&currParamsPoss);
                _modifyFacts(pWhatChanged, factsToModify, pGoalStack, pNow);
              }
            }
            else
            {
              _modifyFacts(pWhatChanged, currInference.factsToModify, pGoalStack, pNow);
            }
          }
          pGoalStack.addGoals(currInference.goalsToAdd, *this, pNow);
          somethingChanged = true;
        }
      }
    }
  }
  return somethingChanged;
}


void WorldState::_notifyWhatChanged(WhatChanged& pWhatChanged,
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
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalStack, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currAddedFact : pWhatChanged.addedFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact.name);
          if (it != condToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalStack, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currRemovedFact : pWhatChanged.removedFacts)
        {
          auto it = notCondToReachableInferences.find(currRemovedFact.name);
          if (it != notCondToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, pGoalStack, it->second, inferences, pNow))
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
    if (pWhatChanged.hasFactsModifications())
      onFactsChanged(_facts);
  }
}


} // !cp
