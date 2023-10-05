#include <contextualplanner/types/problem.hpp>
#include <map>
#include <sstream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/onestepofplannerresult.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{
const int Problem::defaultPriority = 10;


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

}



Problem::Problem(const Problem& pOther)
  : onFactsChanged(),
    onPunctualFacts(),
    onFactsAdded(),
    onFactsRemoved(),
    onGoalsChanged(),
    historical(pOther.historical),
    _goals(pOther._goals),
    _facts(pOther._facts),
    _factNamesToFacts(pOther._factNamesToFacts),
    _accessibleFacts(pOther._accessibleFacts),
    _accessibleFactsWithAnyValues(pOther._accessibleFactsWithAnyValues),
    _removableFacts(pOther._removableFacts),
    _needToAddAccessibleFacts(pOther._needToAddAccessibleFacts),
    _setOfInferences(pOther._setOfInferences),
    _currentGoalPtr(pOther._currentGoalPtr)
{
}


void Problem::notifyActionDone(const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                               const std::unique_ptr<FactModification>& pEffect,
                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                               const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                               const std::vector<Goal>* pGoalsToAddInCurrentPriority)
{
  historical.notifyActionDone(pOnStepOfPlannerResult.actionInstance.actionId);

  WhatChanged whatChanged;
  if (pEffect)
  {
    if (pOnStepOfPlannerResult.actionInstance.parameters.empty())
    {
      _modifyFacts(whatChanged, pEffect, pNow);
    }
    else
    {
      auto effect = pEffect->cloneParamSet(pOnStepOfPlannerResult.actionInstance.parameters);
      _modifyFacts(whatChanged, effect, pNow);
    }
  }

  // Remove current goal if it was one step towards
  int currentPriority = _getCurrentPriority();
  if (pOnStepOfPlannerResult.fromGoal && pOnStepOfPlannerResult.fromGoal->isOneStepTowards())
  {
    auto isNotGoalThatHasDoneOneStepForward = [&](const Goal& pGoal, int){ return pGoal != *pOnStepOfPlannerResult.fromGoal; };
    _iterateOnGoalsAndRemoveNonPersistent(whatChanged, isNotGoalThatHasDoneOneStepForward, pNow);
  }
  else // Else remove only the first goals already satisfied
  {
    _removeFirstGoalsThatAreAlreadySatisfied(whatChanged, pNow);
  }

  if (pGoalsToAdd != nullptr && !pGoalsToAdd->empty())
    _addGoals(whatChanged, *pGoalsToAdd, pNow);
  if (pGoalsToAddInCurrentPriority != nullptr && !pGoalsToAddInCurrentPriority->empty())
    _addGoals(whatChanged, std::map<int, std::vector<cp::Goal>>{{currentPriority, *pGoalsToAddInCurrentPriority}}, pNow);

  _notifyWhatChanged(whatChanged, pNow);
}


bool Problem::addFact(const Fact& pFact,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return addFacts(std::vector<Fact>{pFact}, pNow);
}

template<typename FACTS>
bool Problem::addFacts(const FACTS& pFacts,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _addFacts(whatChanged, pFacts, pNow);
  _notifyWhatChanged(whatChanged, pNow);
  return whatChanged.hasFactsModifications();
}

bool Problem::hasFact(const Fact& pFact) const
{
  return _facts.count(pFact) > 0;
}

bool Problem::removeFact(const Fact& pFact,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  return removeFacts(std::vector<Fact>{pFact}, pNow);
}

template<typename FACTS>
bool Problem::removeFacts(const FACTS& pFacts,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeFacts(whatChanged, pFacts, pNow);
  _notifyWhatChanged(whatChanged, pNow);
  return whatChanged.hasFactsModifications();
}

template bool Problem::addFacts<std::set<Fact>>(const std::set<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template bool Problem::addFacts<std::vector<Fact>>(const std::vector<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);



void Problem::_removeFirstGoalsThatAreAlreadySatisfied(WhatChanged& pWhatChanged,
                                                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  auto alwaysTrue = [&](const Goal&, int){ return true; };
  _iterateOnGoalsAndRemoveNonPersistent(pWhatChanged, alwaysTrue, pNow);
}


void Problem::_iterateOnGoalsAndRemoveNonPersistent(
    WhatChanged& pWhatChanged,
    const std::function<bool(Goal&, int)>& pManageGoal,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  bool isCurrentlyActiveGoal = true;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      // If the goal was inactive for too long we remove it
      if (!isCurrentlyActiveGoal && itGoal->isInactiveForTooLong(pNow))
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        pWhatChanged.goals = true;
        continue;
      }

      if (!isGoalSatisfied(*itGoal))
      {
        _currentGoalPtr = &*itGoal;
        // Check if we are still in this goal
        if (pManageGoal(*itGoal, itGoalsGroup->first))
          return;
        isCurrentlyActiveGoal = false;
      }
      else if (_currentGoalPtr == &*itGoal)
      {
        isCurrentlyActiveGoal = false;
      }

      if (itGoal->isPersistent())
      {
        itGoal->setInactiveSinceIfNotAlreadySet(pNow);
        ++itGoal;
      }
      else
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        pWhatChanged.goals = true;
      }
    }

    if (itGoalsGroup->second.empty())
      itGoalsGroup = _goals.erase(itGoalsGroup);
  }
  // If a goal was activated, but it is not anymore, and we did not find any other active goal
  // then we do not consider anymore the previously activited goal as an activated goal
  if (!isCurrentlyActiveGoal)
    _currentGoalPtr = nullptr;
}


int Problem::_getCurrentPriority() const
{
  bool isCurrentlyActiveGoal = true;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (const auto& currGoal : itGoalsGroup->second)
    {
      if (!currGoal.isPersistent())
        return itGoalsGroup->first;

      if (!isGoalSatisfied(currGoal))
        return itGoalsGroup->first;
    }
  }
  return 0;
}


template<typename FACTS>
void Problem::_addFacts(WhatChanged& pWhatChanged,
                        const FACTS& pFacts,
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
          _removeFacts(pWhatChanged, std::vector<cp::Fact>{currExistingFact}, pNow);
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
      _clearAccessibleAndRemovableFacts();
  }
  _removeNoStackableGoals(pWhatChanged, pNow);
}


template<typename FACTS>
void Problem::_removeFacts(WhatChanged& pWhatChanged,
                           const FACTS& pFacts,
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
    _clearAccessibleAndRemovableFacts();
  }
  _removeNoStackableGoals(pWhatChanged, pNow);
}


template void Problem::_addFacts<std::set<Fact>>(WhatChanged&, const std::set<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template void Problem::_addFacts<std::vector<Fact>>(WhatChanged&, const std::vector<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);


void Problem::_clearAccessibleAndRemovableFacts()
{
  _needToAddAccessibleFacts = true;
  _accessibleFacts.clear();
  _accessibleFactsWithAnyValues.clear();
  _removableFacts.clear();
}

bool Problem::modifyFacts(const std::unique_ptr<FactModification>& pFactModification,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modifyFacts(whatChanged, pFactModification, pNow);
  _notifyWhatChanged(whatChanged, pNow);
  return whatChanged.hasFactsModifications();
}


void Problem::_modifyFacts(WhatChanged& pWhatChanged,
                           const std::unique_ptr<FactModification>& pFactModification,
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

  _addFacts(pWhatChanged, factsToAdd, pNow);
  _removeFacts(pWhatChanged, factsToRemove, pNow);
}

void Problem::setFacts(const std::set<Fact>& pFacts,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _factNamesToFacts.clear();
    for (const auto& currFact : pFacts)
      _factNamesToFacts[currFact.name].insert(currFact);
    _clearAccessibleAndRemovableFacts();
    WhatChanged whatChanged;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}


bool Problem::canFactBecomeTrue(const Fact& pFact) const
{
  if (_facts.count(pFact) > 0 ||
      _accessibleFacts.count(pFact) > 0)
    return true;

  for (const auto& currAccessibleFact : _accessibleFactsWithAnyValues)
    if (pFact.areEqualExceptAnyValues(currAccessibleFact))
      return true;
  return false;
}

std::string Problem::getFactValue(const cp::Fact& pFact) const
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


void Problem::forAllInstruction(const std::string& pParameterName,
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


void Problem::fillAccessibleFacts(const Domain& pDomain)
{
  if (!_needToAddAccessibleFacts)
    return;
  FactsAlreadyChecked factsAlreadychecked;
  for (const auto& currFact : _facts)
  {
    if (_accessibleFacts.count(currFact) == 0)
    {
      auto itPrecToActions = pDomain.preconditionToActions().find(currFact.name);
      if (itPrecToActions != pDomain.preconditionToActions().end())
        _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain,
                                             factsAlreadychecked);
    }
  }
  _feedAccessibleFactsFromSetOfActions(pDomain.actionsWithoutFactToAddInPrecondition(), pDomain,
                                       factsAlreadychecked);
  _needToAddAccessibleFacts = false;
}


void Problem::_feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                                   const Domain& pDomain,
                                                   FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto& actions = pDomain.actions();
  for (const auto& currAction : pActions)
  {
    auto itAction = actions.find(currAction);
    if (itAction != actions.end())
    {
      auto& action = itAction->second;
      _feedAccessibleFactsFromDeduction(action.precondition, action.effect, action.parameters,
                                        pDomain, pFactsAlreadychecked);
    }
  }
}


void Problem::_feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                                      const std::map<InferenceId, Inference>& pAllInferences,
                                                      const Domain& pDomain,
                                                      FactsAlreadyChecked& pFactsAlreadychecked)
{
  for (const auto& currInference : pInferences)
  {
    auto itInference = pAllInferences.find(currInference);
    if (itInference != pAllInferences.end())
    {
      auto& inference = itInference->second;
      std::vector<std::string> parameters;
      _feedAccessibleFactsFromDeduction(inference.condition, inference.factsToModify->clone(nullptr),
                                        parameters, pDomain, pFactsAlreadychecked);
    }
  }
}

void Problem::_feedAccessibleFactsFromDeduction(const std::unique_ptr<FactCondition>& pCondition,
                                                const WorldModification& pEffect,
                                                const std::vector<std::string>& pParameters,
                                                const Domain& pDomain,
                                                FactsAlreadyChecked& pFactsAlreadychecked)
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
          if (factToInsert.replaceParametersByAny(pParameters))
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
        _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked);
      for (const auto& currNewFact : accessibleFactsToAddWithAnyValues)
        _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked);
      for (const auto& currNewFact : removableFactsToAdd)
        _feedAccessibleFactsFromNotFact(currNewFact, pDomain, pFactsAlreadychecked);
    }
  }
}


void Problem::_feedAccessibleFactsFromFact(const Fact& pFact,
                                           const Domain& pDomain,
                                           FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain, pFactsAlreadychecked);

  for (auto& currSetOfInferences : _setOfInferences)
  {
    auto& allInferences = currSetOfInferences.second->inferences();
    auto& conditionToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().conditionToInferences;
    auto itCondToReachableInferences = conditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != conditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, allInferences, pDomain, pFactsAlreadychecked);
    auto& conditionToUnreachableInferences = currSetOfInferences.second->unreachableInferenceLinks().conditionToInferences;
    auto itCondToUnreachableInferences = conditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != conditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, allInferences, pDomain, pFactsAlreadychecked);
  }
}


void Problem::_feedAccessibleFactsFromNotFact(const Fact& pFact,
                                              const Domain& pDomain,
                                              FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToRemove.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.notPreconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.notPreconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain, pFactsAlreadychecked);

  for (auto& currSetOfInferences : _setOfInferences)
  {
    auto& allInferences = currSetOfInferences.second->inferences();
    auto& notConditionToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().notConditionToInferences;
    auto itCondToReachableInferences = notConditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != notConditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, allInferences, pDomain, pFactsAlreadychecked);
    auto& notConditionToUnreachableInferences = currSetOfInferences.second->unreachableInferenceLinks().notConditionToInferences;
    auto itCondToUnreachableInferences = notConditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != notConditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, allInferences, pDomain, pFactsAlreadychecked);
  }
}


std::string Problem::getCurrentGoalStr() const
{
  for (auto itGoalGroup = _goals.rbegin(); itGoalGroup != _goals.rend(); ++itGoalGroup)
    if (!itGoalGroup->second.empty())
      return itGoalGroup->second.front().toStr();
  return "";
}

const Goal* Problem::getCurrentGoalPtr() const
{
  for (auto itGoalGroup = _goals.rbegin(); itGoalGroup != _goals.rend(); ++itGoalGroup)
    if (!itGoalGroup->second.empty())
      return &itGoalGroup->second.front();
  return nullptr;
}


bool Problem::isOptionalFactSatisfied(const FactOptional& pFactOptional) const
{
  return (pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) > 0) &&
      (!pFactOptional.isFactNegated || _facts.count(pFactOptional.fact) == 0);
}


void Problem::iterateOnGoalsAndRemoveNonPersistent(
    const std::function<bool(Goal&, int)>& pManageGoal,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
   WhatChanged whatChanged;
   _iterateOnGoalsAndRemoveNonPersistent(whatChanged, pManageGoal, pNow);
   _notifyWhatChanged(whatChanged, pNow);
}


void Problem::setGoals(const std::map<int, std::vector<Goal>>& pGoals,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_goals != pGoals)
  {
    _currentGoalPtr = nullptr;
    {
      _goals = pGoals;
      WhatChanged whatChanged;
      whatChanged.goals = true;
      _notifyWhatChanged(whatChanged, pNow);
    }
    {
      WhatChanged whatChanged;
      _removeNoStackableGoals(whatChanged, pNow);
      _notifyWhatChanged(whatChanged, pNow);
    }
  }
}

void Problem::setGoals(const std::vector<Goal>& pGoals,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                       int pPriority)
{
  setGoals(std::map<int, std::vector<Goal>>{{pPriority, pGoals}}, pNow);
}

void Problem::addGoals(const std::map<int, std::vector<Goal>>& pGoals,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _addGoals(whatChanged, pGoals, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::addGoals(const std::vector<Goal>& pGoals,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                       int pPriority)
{
  addGoals(std::map<int, std::vector<Goal>>{{pPriority, pGoals}}, pNow);
}


void Problem::_addGoals(WhatChanged& pWhatChanged,
                        const std::map<int, std::vector<Goal>>& pGoals,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (pGoals.empty())
    return;
  {
    // Sub whatChanged object to notify about removed goals
    WhatChanged whatChanged;
    for (auto& currGoals : pGoals)
    {
      auto& existingGoals = _goals[currGoals.first];
      existingGoals.insert(existingGoals.begin(), currGoals.second.begin(), currGoals.second.end());
      whatChanged.goals = true;
    }
    _notifyWhatChanged(whatChanged, pNow);
  }
  _removeNoStackableGoals(pWhatChanged, pNow);
}


void Problem::pushFrontGoal(const Goal& pGoal,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                            int pPriority)
{
  {
    auto& existingGoals = _goals[pPriority];
    existingGoals.insert(existingGoals.begin(), pGoal);

    WhatChanged whatChanged;
    whatChanged.goals = true;
    _notifyWhatChanged(whatChanged, pNow);
  }
  {
    WhatChanged whatChanged;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}

void Problem::pushBackGoal(const Goal& pGoal,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                           int pPriority)
{
  {
    WhatChanged whatChanged;
    auto& existingGoals = _goals[pPriority];
    existingGoals.push_back(pGoal);
    whatChanged.goals = true;
    _notifyWhatChanged(whatChanged, pNow);
  }
  {
    WhatChanged whatChanged;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}


void Problem::changeGoalPriority(const std::string& pGoalStr,
                                 int pPriority,
                                 bool pPushFrontOrBottomInCaseOfConflictWithAnotherGoal,
                                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  std::unique_ptr<Goal> goalToMove;
  WhatChanged whatChanged;
  for (auto itGroup = _goals.begin(); itGroup != _goals.end(); )
  {
    for (auto it = itGroup->second.begin(); it != itGroup->second.end(); )
    {
      if (it->toStr() == pGoalStr)
      {
        goalToMove = std::make_unique<Goal>(std::move(*it));
        itGroup->second.erase(it);
        whatChanged.goals = true;
        break;
      }
      ++it;
    }

    if (itGroup->second.empty())
    {
      itGroup = _goals.erase(itGroup);
      whatChanged.goals = true;
    }
    else
    {
      ++itGroup;
    }

    if (goalToMove)
    {
      auto& goalsForThePriority = _goals[pPriority];
      if (pPushFrontOrBottomInCaseOfConflictWithAnotherGoal)
        goalsForThePriority.insert(goalsForThePriority.begin(), *goalToMove);
      else
        goalsForThePriority.push_back(*goalToMove);
      break;
    }
  }
  _removeNoStackableGoals(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::clearGoals(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_goals.empty())
    return;

  WhatChanged whatChanged;
  _goals.clear();
  whatChanged.goals = true;
  _removeNoStackableGoals(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


bool Problem::removeGoals(const std::string& pGoalGroupId,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  for (auto itGroup = _goals.begin(); itGroup != _goals.end(); )
  {
    for (auto it = itGroup->second.begin(); it != itGroup->second.end(); )
    {
      if (it->getGoalGroupId() == pGoalGroupId)
      {
        it = itGroup->second.erase(it);
        whatChanged.goals = true;
      }
      else
      {
        ++it;
      }
    }

    if (itGroup->second.empty())
      itGroup = _goals.erase(itGroup);
    else
      ++itGroup;
  }
  if (whatChanged.goals)
  {
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
    return true;
  }
  return false;
}


void Problem::removeFirstGoalsThatAreAlreadySatisfied(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeFirstGoalsThatAreAlreadySatisfied(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


bool Problem::isGoalSatisfied(const Goal& pGoal) const
{
  auto* condFactOptPtr = pGoal.conditionFactOptionalPtr();
  return (condFactOptPtr != nullptr && !isOptionalFactSatisfied(*condFactOptPtr)) ||
      pGoal.factCondition().isTrue(*this);
}

bool Problem::isFactPatternSatisfied(const FactOptional& pFactOptional,
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
    bool res = pFactOptional.fact.isInFacts(pRemovedFacts, true, newParameters, pParametersPtr);
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
          factToCompare.fillParameters(currParamPoss);
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
    if (pFactOptional.fact.isInFacts(_facts, true, newParameters, pParametersPtr, false, &triedToMidfyParameters))
    {
      if (pCanBecomeTruePtr != nullptr && triedToMidfyParameters)
        *pCanBecomeTruePtr = true;
      return false;
    }
    return true;
  }

  auto res = pFactOptional.fact.isInFacts(_facts, true, newParameters, pParametersPtr);
  if (pParametersPtr != nullptr)
    applyNewParams(*pParametersPtr, newParameters);
  return res;
}

std::map<int, std::vector<Goal>> Problem::getNotSatisfiedGoals() const
{
  std::map<int, std::vector<Goal>> res;
  for (auto& currGoalWithPriority : _goals)
    for (auto& currGoal : currGoalWithPriority.second)
      if (!isGoalSatisfied(currGoal))
        res[currGoalWithPriority.first].push_back(currGoal);
  return res;
}

void Problem::addSetOfInferences(const SetOfInferencesId& pSetOfInferencesId,
                                 const std::shared_ptr<const SetOfInferences>& pSetOfInferences)
{
  _setOfInferences.emplace(pSetOfInferencesId, pSetOfInferences);
  _clearAccessibleAndRemovableFacts();
}

void Problem::removeSetOfInferences(const SetOfInferencesId& pSetOfInferencesId)
{
  auto it = _setOfInferences.find(pSetOfInferencesId);
  if (it != _setOfInferences.end())
    _setOfInferences.erase(it);
  _clearAccessibleAndRemovableFacts();
}

void Problem::clearInferences()
{
  _setOfInferences.clear();
  _clearAccessibleAndRemovableFacts();
}


void Problem::_removeNoStackableGoals(WhatChanged& pWhatChanged,
                                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  bool firstGoal = true;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      if (isGoalSatisfied(*itGoal))
      {
        ++itGoal;
        continue;
      }

      if (firstGoal)
      {
        firstGoal = false;
        ++itGoal;
        continue;
      }

      if (!itGoal->isInactiveForTooLong(pNow))
      {
        itGoal->setInactiveSinceIfNotAlreadySet(pNow);
        ++itGoal;
      }
      else
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        pWhatChanged.goals = true;
      }
    }

    if (itGoalsGroup->second.empty())
      itGoalsGroup = _goals.erase(itGoalsGroup);
  }
}


bool Problem::_tryToApplyInferences(std::set<InferenceId>& pInferencesAlreadyApplied,
                                    WhatChanged& pWhatChanged,
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
                _modifyFacts(pWhatChanged, factsToModify, pNow);
              }
            }
            else
            {
              _modifyFacts(pWhatChanged, currInference.factsToModify, pNow);
            }
          }
          _addGoals(pWhatChanged, currInference.goalsToAdd, pNow);
          somethingChanged = true;
        }
      }
    }
  }
  return somethingChanged;
}


void Problem::_notifyWhatChanged(WhatChanged& pWhatChanged,
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
      for (auto& currSetOfInferences : _setOfInferences)
      {
        auto& inferences = currSetOfInferences.second->inferences();
        auto& condToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().conditionToInferences;
        auto& notCondToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().notConditionToInferences;
        auto& inferencesAlreadyApplied = soiToInferencesAlreadyApplied[currSetOfInferences.first];

        for (auto& currAddedFact : pWhatChanged.punctualFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact.name);
          if (it != condToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currAddedFact : pWhatChanged.addedFacts)
        {
          auto it = condToReachableInferences.find(currAddedFact.name);
          if (it != condToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, it->second, inferences, pNow))
            needAnotherLoop = true;
        }
        for (auto& currRemovedFact : pWhatChanged.removedFacts)
        {
          auto it = notCondToReachableInferences.find(currRemovedFact.name);
          if (it != notCondToReachableInferences.end() &&
              _tryToApplyInferences(inferencesAlreadyApplied, pWhatChanged, it->second, inferences, pNow))
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
    if (pWhatChanged.goals)
      onGoalsChanged(_goals);
  }
}


} // !cp
