#include <contextualplanner/types/problem.hpp>
#include <sstream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/setofinferences.hpp>

namespace cp
{
const int Problem::defaultPriority = 10;


namespace
{

template <typename T>
T _lexical_cast(const std::string& pStr)
{
  bool firstChar = true;
  for (const auto& currChar : pStr)
  {
    if ((currChar < '0' || currChar > '9') &&
        !(firstChar && currChar == '-'))
      throw std::runtime_error("bad lexical cast: source type value could not be interpreted as target");
    firstChar = false;
  }
  return atoi(pStr.c_str());
}

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
      ss << _lexical_cast<int>(pStr) + 1;
      pStr = ss.str();
    }
    catch (...) {}
  }
}

void _getTheFactsToAddFromAWorldModification(std::set<Fact>& pNewFacts,
                                             std::vector<Fact>& pNewFactsWithAnyValues,
                                             const WorldModification& pWorldModification,
                                             const std::vector<std::string>& pParameters,
                                             const std::set<Fact>& pFacts1,
                                             const std::set<Fact>& pFacts2)
{
  pWorldModification.forAllFacts([&](const cp::Fact& pFact) {
    if (pFacts1.count(pFact) == 0 &&
        pFacts2.count(pFact) == 0)
    {
      auto factToInsert = pFact;
      if (factToInsert.replaceParametersByAny(pParameters))
        pNewFactsWithAnyValues.push_back(std::move(factToInsert));
      else
        pNewFacts.insert(std::move(factToInsert));
    }
  });
}

void _getTheFactsToRemoveFromAWorldModification(std::set<Fact>& pFactsToRemove,
                                                const WorldModification& pWorldModification,
                                                const std::set<Fact>& pFacts1,
                                                const std::set<Fact>& pFacts2)
{
  pWorldModification.forAllNotFacts([&](const cp::Fact& pNotFact) {
    if (pFacts1.count(pNotFact) > 0 &&
        pFacts2.count(pNotFact) == 0)
      pFactsToRemove.insert(pNotFact);
  });
}

}



Problem::Problem(const Problem& pOther)
  : onVariablesToValueChanged(),
    onFactsChanged(),
    onGoalsChanged(),
    historical(pOther.historical),
    _goals(pOther._goals),
    _variablesToValue(pOther._variablesToValue),
    _facts(pOther._facts),
    _factNamesToNbOfOccurences(pOther._factNamesToNbOfOccurences),
    _accessibleFacts(pOther._accessibleFacts),
    _accessibleFactsWithAnyValues(pOther._accessibleFactsWithAnyValues),
    _removableFacts(pOther._removableFacts),
    _needToAddAccessibleFacts(pOther._needToAddAccessibleFacts),
    _setOfInferences(pOther._setOfInferences)
{
}


void Problem::notifyActionDone(const std::string& pActionId,
                               const std::map<std::string, std::string>& pParameters,
                               const SetOfFacts& pEffect,
                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                               const std::map<int, std::vector<Goal>>* pGoalsToAdd)
{
  historical.notifyActionDone(pActionId);
  WhatChanged whatChanged;
  if (pParameters.empty())
  {
    _modifyFacts(whatChanged, pEffect, pNow);
  }
  else
  {
    cp::SetOfFacts effect;
    effect.notFacts = pEffect.notFacts;
    effect.exps = pEffect.exps;
    for (auto& currFact : pEffect.facts)
    {
      auto fact = currFact;
      fact.fillParameters(pParameters);
      effect.facts.insert(std::move(fact));
    }
    _modifyFacts(whatChanged, effect, pNow);
  }
  if (pGoalsToAdd != nullptr && !pGoalsToAdd->empty())
    _addGoals(whatChanged, *pGoalsToAdd, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue)
{
  if (!pVariablesToValue.empty())
    for (const auto& currFactToVal : pVariablesToValue)
      _variablesToValue[currFactToVal.first] = currFactToVal.second;
  onVariablesToValueChanged(_variablesToValue);
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


void Problem::_addFactNameRef(const std::string& pFactName)
{
  auto itFactName = _factNamesToNbOfOccurences.find(pFactName);
  if (itFactName == _factNamesToNbOfOccurences.end())
    _factNamesToNbOfOccurences[pFactName] = 1;
  else
    ++itFactName->second;
}

template<typename FACTS>
void Problem::_addFacts(WhatChanged& pWhatChanged,
                        const FACTS& pFacts,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.isUnreachable())
      break;
    if (currFact.isPunctual())
    {
      pWhatChanged.punctualFacts.insert(currFact);
      break;
    }
    if (_facts.count(currFact) > 0)
      continue;
    pWhatChanged.addedFacts.insert(currFact);
    _facts.insert(currFact);
    _addFactNameRef(currFact.name);

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
      continue;
    pWhatChanged.removedFacts.insert(currFact);
    _facts.erase(it);
    {
      auto itFactName = _factNamesToNbOfOccurences.find(currFact.name);
      if (itFactName == _factNamesToNbOfOccurences.end())
        assert(false);
      else if (itFactName->second == 1)
        _factNamesToNbOfOccurences.erase(itFactName);
      else
        --itFactName->second;
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

bool Problem::modifyFacts(const SetOfFacts& pSetOfFacts,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modifyFacts(whatChanged, pSetOfFacts, pNow);
  _notifyWhatChanged(whatChanged, pNow);
  return whatChanged.hasFactsModifications();
}


void Problem::_modifyFacts(WhatChanged& pWhatChanged,
                           const SetOfFacts& pSetOfFacts,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  _addFacts(pWhatChanged, pSetOfFacts.facts, pNow);
  _removeFacts(pWhatChanged, pSetOfFacts.notFacts, pNow);
  bool variablesToValueChanged = false;
  for (auto& currExp : pSetOfFacts.exps)
  {
    if (currExp.elts.size() >= 2)
    {
      auto it = currExp.elts.begin();
      if (it->type == ExpressionElementType::OPERATOR)
      {
        auto op = it->value;
        ++it;
        if (op == "++" &&
            it->type == ExpressionElementType::FACT)
        {
          _incrementStr(_variablesToValue[it->value]);
          variablesToValueChanged = true;
        }
      }
      else if (it->type == ExpressionElementType::FACT)
      {
        auto factToSet = it->value;
        ++it;
        if (it->type == ExpressionElementType::OPERATOR)
        {
          auto op = it->value;
          ++it;
          if (op == "=" &&
              it->type == ExpressionElementType::VALUE)
          {
            _variablesToValue[factToSet] = it->value;
            variablesToValueChanged = true;
          }
        }
      }
    }
  }
  if (variablesToValueChanged)
    onVariablesToValueChanged(_variablesToValue);
}

void Problem::setFacts(const std::set<Fact>& pFacts,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_facts != pFacts)
  {
    _facts = pFacts;
    _factNamesToNbOfOccurences.clear();
    for (const auto& currFact : pFacts)
      _addFactNameRef(currFact.name);
    _clearAccessibleAndRemovableFacts();
    WhatChanged whatChanged;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}


bool Problem::canSetOfFactsBecomeTrue(const SetOfFacts& pSetOfFacts) const
{
  if (!canFactsBecomeTrue(pSetOfFacts.facts))
    return false;
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (_facts.count(currFact) > 0 &&
        _removableFacts.count(currFact) == 0)
      return false;
  return areExpsValid(pSetOfFacts.exps, _variablesToValue);
}

bool Problem::canFactsBecomeTrue(const std::set<Fact>& pFacts) const
{
  for (const auto& currFact : pFacts)
  {
    if (_facts.count(currFact) == 0 &&
        _accessibleFacts.count(currFact) == 0)
    {
      bool reableFactFound = false;
      for (const auto& currAccessibleFact : _accessibleFactsWithAnyValues)
      {
        if (currFact.areEqualExceptAnyValues(currAccessibleFact))
        {
          reableFactFound = true;
          break;
        }
      }
      if (!reableFactFound)
        return false;
    }
  }
  return true;
}

bool Problem::areFactsTrue(const SetOfFacts& pSetOfFacts,
                           const std::set<Fact>& pPunctualFacts,
                           std::map<std::string, std::string>* pParametersPtr) const
{
  for (const auto& currFact : pSetOfFacts.facts)
  {
    if (currFact.isPunctual())
    {
      if (pPunctualFacts.count(currFact) == 0)
        return false;
    }
    else
    {
      if (!currFact.isInFacts(_facts, true, pParametersPtr))
        return false;
    }
  }
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (currFact.isInFacts(_facts, true, pParametersPtr))
      return false;
  return areExpsValid(pSetOfFacts.exps, _variablesToValue);
}


void Problem::fillAccessibleFacts(const Domain& pDomain)
{
  if (!_needToAddAccessibleFacts)
    return;
  for (const auto& currFact : _facts)
  {
    if (_accessibleFacts.count(currFact) == 0)
    {
      auto itPrecToActions = pDomain.preconditionToActions().find(currFact.name);
      if (itPrecToActions != pDomain.preconditionToActions().end())
        _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain);
    }
  }
  _feedAccessibleFactsFromSetOfActions(pDomain.actionsWithoutFactToAddInPrecondition(), pDomain);
  _needToAddAccessibleFacts = false;
}


void Problem::_feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                                  const Domain& pDomain)
{
  auto& actions = pDomain.actions();
  for (const auto& currAction : pActions)
  {
    auto itAction = actions.find(currAction);
    if (itAction != actions.end())
    {
      auto& action = itAction->second;
      _feedAccessibleFactsFromDeduction(action.preconditions, action.effect,
                                       action.parameters, pDomain);
    }
  }
}


void Problem::_feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                                      const Domain& pDomain)
{
  if (_setOfInferences)
  {
    auto& inferences = _setOfInferences->inferences();
    for (const auto& currInference : pInferences)
    {
      auto itInference = inferences.find(currInference);
      if (itInference != inferences.end())
      {
        auto& inference = itInference->second;
        std::vector<std::string> parameters;
        _feedAccessibleFactsFromDeduction(inference.condition, inference.factsToModify,
                                          parameters, pDomain);
      }
    }
  }
}

void Problem::_feedAccessibleFactsFromDeduction(const SetOfFacts& pCondition,
                                               const WorldModification& pEffect,
                                               const std::vector<std::string>& pParameters,
                                               const Domain& pDomain)
{
  if (canSetOfFactsBecomeTrue(pCondition))
  {
    std::set<Fact> accessibleFactsToAdd;
    std::vector<Fact> accessibleFactsToAddWithAnyValues;
    _getTheFactsToAddFromAWorldModification(accessibleFactsToAdd, accessibleFactsToAddWithAnyValues,
                                            pEffect, pParameters, _facts, _accessibleFacts);
    std::set<Fact> removableFactsToAdd;
    _getTheFactsToRemoveFromAWorldModification(removableFactsToAdd, pEffect, _facts, _removableFacts);
    if (!accessibleFactsToAdd.empty() || !accessibleFactsToAddWithAnyValues.empty() || !removableFactsToAdd.empty())
    {
      _accessibleFacts.insert(accessibleFactsToAdd.begin(), accessibleFactsToAdd.end());
      _accessibleFactsWithAnyValues.insert(accessibleFactsToAddWithAnyValues.begin(), accessibleFactsToAddWithAnyValues.end());
      _removableFacts.insert(removableFactsToAdd.begin(), removableFactsToAdd.end());
      for (const auto& currNewFact : accessibleFactsToAdd)
        _feedAccessibleFactsFromFact(currNewFact, pDomain);
      for (const auto& currNewFact : removableFactsToAdd)
        _feedAccessibleFactsFromNotFact(currNewFact, pDomain);
    }
  }
}


void Problem::_feedAccessibleFactsFromFact(const Fact& pFact,
                                           const Domain& pDomain)
{
  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain);

  if (_setOfInferences)
  {
    auto& conditionToReachableInferences = _setOfInferences->reachableInferenceLinks().conditionToInferences;
    auto itCondToReachableInferences = conditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != conditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, pDomain);
    auto& conditionToUnreachableInferences = _setOfInferences->unreachableInferenceLinks().conditionToInferences;
    auto itCondToUnreachableInferences = conditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != conditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, pDomain);
  }
}


void Problem::_feedAccessibleFactsFromNotFact(const Fact& pFact,
                                              const Domain& pDomain)
{
  auto itPrecToActions = pDomain.notPreconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.notPreconditionToActions().end())
    _feedAccessibleFactsFromSetOfActions(itPrecToActions->second, pDomain);

  if (_setOfInferences)
  {
    auto& notConditionToReachableInferences = _setOfInferences->reachableInferenceLinks().notConditionToInferences;
    auto itCondToReachableInferences = notConditionToReachableInferences.find(pFact.name);
    if (itCondToReachableInferences != notConditionToReachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences->second, pDomain);
    auto& notConditionToUnreachableInferences = _setOfInferences->unreachableInferenceLinks().notConditionToInferences;
    auto itCondToUnreachableInferences = notConditionToUnreachableInferences.find(pFact.name);
    if (itCondToUnreachableInferences != notConditionToUnreachableInferences.end())
      _feedAccessibleFactsFromSetOfInferences(itCondToUnreachableInferences->second, pDomain);
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
  bool firstGoal = true;
  WhatChanged whatChanged;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      bool wasInactiveForTooLong = firstGoal ? false : itGoal->isInactiveForTooLong(pNow);

      auto* goalConditionFactPtr = itGoal->conditionFactOptionalPtr();
      if (goalConditionFactPtr == nullptr ||
          isOptionalFactSatisfied(*goalConditionFactPtr))
      {
        firstGoal = false;
        if (!wasInactiveForTooLong && pManageGoal(*itGoal, itGoalsGroup->first))
        {
          _notifyWhatChanged(whatChanged, pNow);
          return;
        }
      }

      if (itGoal->isPersistent() && !wasInactiveForTooLong)
      {
        itGoal->setInactiveSinceIfNotAlreadySet(pNow);
        ++itGoal;
      }
      else
      {
        itGoal = itGoalsGroup->second.erase(itGoal);
        whatChanged.goals = true;
      }
    }

    if (itGoalsGroup->second.empty())
      itGoalsGroup = _goals.erase(itGoalsGroup);
  }
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::setGoals(const std::map<int, std::vector<Goal>>& pGoals,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_goals != pGoals)
  {
    _goals = pGoals;
    WhatChanged whatChanged;
    whatChanged.goals = true;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
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
  for (auto& currGoals : pGoals)
  {
    auto& existingGoals = _goals[currGoals.first];
    existingGoals.insert(existingGoals.begin(), currGoals.second.begin(), currGoals.second.end());
    pWhatChanged.goals = true;
  }
  _removeNoStackableGoals(pWhatChanged, pNow);
}


void Problem::pushFrontGoal(const Goal& pGoal,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                            int pPriority)
{
  auto& existingGoals = _goals[pPriority];
  existingGoals.insert(existingGoals.begin(), pGoal);
  WhatChanged whatChanged;
  _removeNoStackableGoals(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}

void Problem::pushBackGoal(const Goal& pGoal,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                           int pPriority)
{
  auto& existingGoals = _goals[pPriority];
  existingGoals.push_back(pGoal);
  WhatChanged whatChanged;
  _removeNoStackableGoals(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
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
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::removeGoals(const std::string& pGoalGroupId,
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
    _removeNoStackableGoals(whatChanged, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void Problem::removeFirstGoalsThatAreAlreadySatisfied()
{
  auto isGoalNotAlreadySatisfied = [&](const Goal& pGoal, int){
    return !isOptionalFactSatisfied(pGoal.factOptional());
  };

  iterateOnGoalsAndRemoveNonPersistent(isGoalNotAlreadySatisfied, {});
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
      if (itGoal->conditionFactOptionalPtr() != nullptr && !isOptionalFactSatisfied(*itGoal->conditionFactOptionalPtr()))
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
        if (areFactsTrue(currInference.condition, pWhatChanged.punctualFacts))
        {
          _modifyFacts(pWhatChanged, currInference.factsToModify, pNow);
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
    if (_setOfInferences)
    {
      auto& inferences = _setOfInferences->inferences();
      auto& condToReachableInferences = _setOfInferences->reachableInferenceLinks().conditionToInferences;
      auto& notCondToReachableInferences = _setOfInferences->reachableInferenceLinks().notConditionToInferences;
      std::set<InferenceId> inferencesAlreadyApplied;
      bool needAnotherLoop = true;
      while (needAnotherLoop)
      {
        needAnotherLoop = false;

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
