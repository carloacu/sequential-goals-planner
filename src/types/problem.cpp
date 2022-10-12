#include <contextualplanner/types/problem.hpp>
#include <sstream>
#include <contextualplanner/types/domain.hpp>

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

void _getTheFactsToAddFromTheActionEffects(std::set<Fact>& pNewFacts,
                                           std::vector<Fact>& pNewFactsWithAnyValues,
                                           const Action& pAction,
                                           const std::set<Fact>& pFacts1,
                                           const std::set<Fact>& pFacts2)
{
  pAction.effect.forAllFacts([&](const cp::Fact& pFact) {
    if (pFacts1.count(pFact) == 0 &&
        pFacts2.count(pFact) == 0)
    {
      auto factToInsert = pFact;
      if (factToInsert.replaceParametersByAny(pAction.parameters))
        pNewFactsWithAnyValues.push_back(std::move(factToInsert));
      else
        pNewFacts.insert(std::move(factToInsert));
    }
  });
}

void _getTheFactsToRemoveFromTheActionEffects(std::set<Fact>& pFactsToRemove,
                                              const Action& pAction,
                                              const std::set<Fact>& pFacts1,
                                              const std::set<Fact>& pFacts2)
{
  pAction.effect.forAllNotFacts([&](const cp::Fact& pNotFact) {
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
    _reachableFacts(pOther._reachableFacts),
    _reachableFactsWithAnyValues(pOther._reachableFactsWithAnyValues),
    _removableFacts(pOther._removableFacts),
    _needToAddReachableFacts(pOther._needToAddReachableFacts)
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
  return whatChanged.facts;
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
  return whatChanged.facts;
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
    if (_facts.count(currFact) > 0)
      continue;
    pWhatChanged.facts = true;
    _facts.insert(currFact);
    _addFactNameRef(currFact.name);

    auto itReachable = _reachableFacts.find(currFact);
    if (itReachable != _reachableFacts.end())
      _reachableFacts.erase(itReachable);
    else
      _clearReachableAndRemovableFacts();
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
    pWhatChanged.facts = true;
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
    _clearReachableAndRemovableFacts();
  }
  _removeNoStackableGoals(pWhatChanged, pNow);
}


template void Problem::_addFacts<std::set<Fact>>(WhatChanged&, const std::set<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);
template void Problem::_addFacts<std::vector<Fact>>(WhatChanged&, const std::vector<Fact>&, const std::unique_ptr<std::chrono::steady_clock::time_point>&);


void Problem::_clearReachableAndRemovableFacts()
{
  _needToAddReachableFacts = true;
  _reachableFacts.clear();
  _reachableFactsWithAnyValues.clear();
  _removableFacts.clear();
}

bool Problem::modifyFacts(const SetOfFacts& pSetOfFacts,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _modifyFacts(whatChanged, pSetOfFacts, pNow);
  _notifyWhatChanged(whatChanged, pNow);
  return whatChanged.facts;
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
    _clearReachableAndRemovableFacts();
    WhatChanged whatChanged;
    _removeNoStackableGoals(whatChanged, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}


bool Problem::canFactsBecomeTrue(const SetOfFacts& pSetOfFacts) const
{
  for (const auto& currFact : pSetOfFacts.facts)
  {
    if (_facts.count(currFact) == 0 &&
        _reachableFacts.count(currFact) == 0)
    {
      bool reableFactFound = false;
      for (const auto& currReachableFact : _reachableFactsWithAnyValues)
      {
        if (currFact.areEqualExceptAnyValues(currReachableFact))
        {
          reableFactFound = true;
          break;
        }
      }
      if (!reableFactFound)
        return false;
    }
  }
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (_facts.count(currFact) > 0 &&
        _removableFacts.count(currFact) == 0)
      return false;
  return areExpsValid(pSetOfFacts.exps, _variablesToValue);
}


bool Problem::areFactsTrue(const SetOfFacts& pSetOfFacts,
                           std::map<std::string, std::string>* pParametersPtr) const
{
  //auto& facts = pProblem.facts();
  for (const auto& currFact : pSetOfFacts.facts)
    if (!currFact.isInFacts(_facts, true, pParametersPtr))
      return false;
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (currFact.isInFacts(_facts, true, pParametersPtr))
      return false;
  return areExpsValid(pSetOfFacts.exps, _variablesToValue);
}


void Problem::fillReachableFacts(const Domain& pDomain)
{
  if (!_needToAddReachableFacts)
    return;
  for (const auto& currFact : _facts)
  {
    if (_reachableFacts.count(currFact) == 0)
      _feedReachableFacts(currFact, pDomain);
  }
  _feedReachableFactsFromSetOfActions(pDomain.actionsWithoutPrecondition(), pDomain);
  _needToAddReachableFacts = false;
}


void Problem::_feedReachableFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                                  const Domain& pDomain)
{
  for (const auto& currAction : pActions)
  {
    auto itAction = pDomain.actions().find(currAction);
    if (itAction != pDomain.actions().end())
    {
      auto& action = itAction->second;
      if (canFactsBecomeTrue(action.preconditions))
      {
        std::set<Fact> reachableFactsToAdd;
        std::vector<Fact> reachableFactsToAddWithAnyValues;
        _getTheFactsToAddFromTheActionEffects(reachableFactsToAdd, reachableFactsToAddWithAnyValues, action, _facts, _reachableFacts);
        std::set<Fact> removableFactsToAdd;
        _getTheFactsToRemoveFromTheActionEffects(removableFactsToAdd, action, _facts, _removableFacts);
        if (!reachableFactsToAdd.empty() || !reachableFactsToAddWithAnyValues.empty() || !removableFactsToAdd.empty())
        {
          _reachableFacts.insert(reachableFactsToAdd.begin(), reachableFactsToAdd.end());
          _reachableFactsWithAnyValues.insert(reachableFactsToAddWithAnyValues.begin(), reachableFactsToAddWithAnyValues.end());
          _removableFacts.insert(removableFactsToAdd.begin(), removableFactsToAdd.end());
          for (const auto& currNewFact : reachableFactsToAdd)
            _feedReachableFacts(currNewFact, pDomain);
          for (const auto& currNewFact : removableFactsToAdd)
            _feedReachableFacts(currNewFact, pDomain);
        }
      }
    }
  }
}


void Problem::_feedReachableFacts(const Fact& pFact,
                                  const Domain& pDomain)
{
  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedReachableFactsFromSetOfActions(itPrecToActions->second, pDomain);
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

      auto* goalConditionFactPtr = itGoal->conditionFactPtr();
      if (goalConditionFactPtr == nullptr ||
          _facts.count(*goalConditionFactPtr) > 0)
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
    auto& goalFact = pGoal.fact();
    return _facts.count(goalFact) == 0;
  };

  iterateOnGoalsAndRemoveNonPersistent(isGoalNotAlreadySatisfied, {});
}


void Problem::setInferences(const std::list<Inference>& pInferences)
{
  _inferences = pInferences;
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
      if (itGoal->conditionFactPtr() != nullptr && _facts.count(*itGoal->conditionFactPtr()) == 0)
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

void Problem::_notifyWhatChanged(WhatChanged& pWhatChanged,
                                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (pWhatChanged.facts || pWhatChanged.goals)
  {
    // manage the inferences
    std::set<const Inference*> inferencesAlreadyApplied;
    bool needAnotherLoop = true;
    while (needAnotherLoop)
    {
      needAnotherLoop = false;
      for (const auto& currInference : _inferences)
      {
        if (inferencesAlreadyApplied.count(&currInference) == 0 &&
            areFactsTrue(currInference.condition))
        {
          inferencesAlreadyApplied.insert(&currInference);
          _modifyFacts(pWhatChanged, currInference.factsToModify, pNow);
          _addGoals(pWhatChanged, currInference.goalsToAdd, pNow);
          needAnotherLoop = true;
        }
      }
    }

    if (pWhatChanged.facts)
      onFactsChanged(_facts);
    if (pWhatChanged.goals)
      onGoalsChanged(_goals);
  }

}



} // !cp
