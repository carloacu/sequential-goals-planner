#include <contextualplanner/types/goalstack.hpp>
#include <map>
#include <sstream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/onestepofplannerresult.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>
#include <contextualplanner/types/lookforanactionoutputinfos.hpp>


namespace cp
{
const int GoalStack::defaultPriority = 10;



GoalStack::GoalStack(const GoalStack& pOther)
  : onGoalsChanged(),
    _goals(pOther._goals),
    _currentGoalPtr(pOther._currentGoalPtr)
{
}


void GoalStack::notifyActionDone(const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                                 const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                                 const std::vector<Goal>* pGoalsToAddInCurrentPriority,
                                 const WorldState& pWorldState,
                                 LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  WhatChanged whatChanged;

  // Remove current goal if it was one step towards
  int currentPriority = _getCurrentPriority(pWorldState);
  if (pOnStepOfPlannerResult.fromGoal && pOnStepOfPlannerResult.fromGoal->isOneStepTowards())
  {
    auto isNotGoalThatHasDoneOneStepForward = [&](const Goal& pGoal, int){ return pGoal != *pOnStepOfPlannerResult.fromGoal; };
    _iterateOnGoalsAndRemoveNonPersistent(whatChanged, isNotGoalThatHasDoneOneStepForward, pWorldState, pNow, pLookForAnActionOutputInfosPtr);
  }
  else // Else remove only the first goals already satisfied
  {
    _removeFirstGoalsThatAreAlreadySatisfied(whatChanged, pWorldState, pNow, pLookForAnActionOutputInfosPtr);
  }

  if (pGoalsToAdd != nullptr && !pGoalsToAdd->empty())
    _addGoals(whatChanged, *pGoalsToAdd, pWorldState, pNow);
  if (pGoalsToAddInCurrentPriority != nullptr && !pGoalsToAddInCurrentPriority->empty())
    _addGoals(whatChanged, std::map<int, std::vector<cp::Goal>>{{currentPriority, *pGoalsToAddInCurrentPriority}}, pWorldState, pNow);

  _notifyWhatChanged(whatChanged, pNow);
}


void GoalStack::_removeFirstGoalsThatAreAlreadySatisfied(WhatChanged& pWhatChanged,
                                                         const WorldState& pWorldState,
                                                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                                                         LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  auto alwaysTrue = [&](const Goal&, int){ return true; };
  _iterateOnGoalsAndRemoveNonPersistent(pWhatChanged, alwaysTrue, pWorldState, pNow, pLookForAnActionOutputInfosPtr);
}


void GoalStack::_iterateOnGoalsAndRemoveNonPersistent(
    WhatChanged& pWhatChanged,
    const std::function<bool(Goal&, int)>& pManageGoal,
    const WorldState& pWorldState,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  if (pLookForAnActionOutputInfosPtr != nullptr)
    pLookForAnActionOutputInfosPtr->setType(PlannerStepType::FINISHED_ON_SUCCESS);

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

      if (!pWorldState.isGoalSatisfied(*itGoal))
      {
        _currentGoalPtr = &*itGoal;
        // Check if we are still in this goal
        if (pManageGoal(*itGoal, itGoalsGroup->first))
        {
          if (pLookForAnActionOutputInfosPtr != nullptr)
            pLookForAnActionOutputInfosPtr->setType(PlannerStepType::IN_PROGRESS);
          return;
        }
        isCurrentlyActiveGoal = false;

        if (pLookForAnActionOutputInfosPtr != nullptr)
          pLookForAnActionOutputInfosPtr->setType(PlannerStepType::FINISEHD_ON_FAILURE);
      }
      else
      {
        if (pLookForAnActionOutputInfosPtr != nullptr && pLookForAnActionOutputInfosPtr->getType() != PlannerStepType::FINISEHD_ON_FAILURE)
          pLookForAnActionOutputInfosPtr->setType(PlannerStepType::FINISHED_ON_SUCCESS);

        if (pLookForAnActionOutputInfosPtr != nullptr)
          pLookForAnActionOutputInfosPtr->notifySatisfiedGoal(*itGoal);
        if (_currentGoalPtr == &*itGoal)
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


int GoalStack::_getCurrentPriority(const WorldState& pWorldState) const
{
  bool isCurrentlyActiveGoal = true;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (const auto& currGoal : itGoalsGroup->second)
    {
      if (!currGoal.isPersistent())
        return itGoalsGroup->first;

      if (!pWorldState.isGoalSatisfied(currGoal))
        return itGoalsGroup->first;
    }
  }
  return 0;
}




std::string GoalStack::getCurrentGoalStr() const
{
  for (auto itGoalGroup = _goals.rbegin(); itGoalGroup != _goals.rend(); ++itGoalGroup)
    if (!itGoalGroup->second.empty())
      return itGoalGroup->second.front().toStr();
  return "";
}

const Goal* GoalStack::getCurrentGoalPtr() const
{
  for (auto itGoalGroup = _goals.rbegin(); itGoalGroup != _goals.rend(); ++itGoalGroup)
    if (!itGoalGroup->second.empty())
      return &itGoalGroup->second.front();
  return nullptr;
}



void GoalStack::iterateOnGoalsAndRemoveNonPersistent(
    const std::function<bool(Goal&, int)>& pManageGoal,
    const WorldState& pWorldState,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  WhatChanged whatChanged;
  _iterateOnGoalsAndRemoveNonPersistent(whatChanged, pManageGoal, pWorldState, pNow,
                                        pLookForAnActionOutputInfosPtr);
  _notifyWhatChanged(whatChanged, pNow);
}


void GoalStack::setGoals(const std::map<int, std::vector<Goal>>& pGoals,
                         const WorldState& pWorldState,
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
      _removeNoStackableGoals(whatChanged, pWorldState, pNow);
      _notifyWhatChanged(whatChanged, pNow);
    }
  }
}

void GoalStack::setGoals(const std::vector<Goal>& pGoals,
                         const WorldState& pWorldState,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                         int pPriority)
{
  setGoals(std::map<int, std::vector<Goal>>{{pPriority, pGoals}}, pWorldState, pNow);
}

void GoalStack::addGoals(const std::map<int, std::vector<Goal>>& pGoals,
                         const WorldState& pWorldState,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _addGoals(whatChanged, pGoals, pWorldState, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void GoalStack::addGoals(const std::vector<Goal>& pGoals,
                         const WorldState& pWorldState,
                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                         int pPriority)
{
  addGoals(std::map<int, std::vector<Goal>>{{pPriority, pGoals}}, pWorldState, pNow);
}


void GoalStack::_addGoals(WhatChanged& pWhatChanged,
                          const std::map<int, std::vector<Goal>>& pGoals,
                          const WorldState& pWorldState,
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
  _removeNoStackableGoals(pWhatChanged, pWorldState, pNow);
}


void GoalStack::pushFrontGoal(const Goal& pGoal,
                              const WorldState& pWorldState,
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
    _removeNoStackableGoals(whatChanged, pWorldState, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}

void GoalStack::pushBackGoal(const Goal& pGoal,
                             const WorldState& pWorldState,
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
    _removeNoStackableGoals(whatChanged, pWorldState, pNow);
    _notifyWhatChanged(whatChanged, pNow);
  }
}


void GoalStack::changeGoalPriority(const std::string& pGoalStr,
                                   int pPriority,
                                   bool pPushFrontOrBottomInCaseOfConflictWithAnotherGoal,
                                   const WorldState& pWorldState,
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
  _removeNoStackableGoals(whatChanged, pWorldState, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


void GoalStack::clearGoals(const WorldState& pWorldState,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (_goals.empty())
    return;

  WhatChanged whatChanged;
  _goals.clear();
  whatChanged.goals = true;
  _removeNoStackableGoals(whatChanged, pWorldState, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}


bool GoalStack::removeGoals(const std::string& pGoalGroupId,
                            const WorldState& pWorldState,
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
    _removeNoStackableGoals(whatChanged, pWorldState, pNow);
    _notifyWhatChanged(whatChanged, pNow);
    return true;
  }
  return false;
}


void GoalStack::removeFirstGoalsThatAreAlreadySatisfied(const WorldState& pWorldState,
                                                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeFirstGoalsThatAreAlreadySatisfied(whatChanged, pWorldState, pNow, nullptr);
  _notifyWhatChanged(whatChanged, pNow);
}



std::map<int, std::vector<Goal>> GoalStack::getNotSatisfiedGoals(const WorldState& pWorldState) const
{
  std::map<int, std::vector<Goal>> res;
  for (auto& currGoalWithPriority : _goals)
    for (auto& currGoal : currGoalWithPriority.second)
      if (!pWorldState.isGoalSatisfied(currGoal))
        res[currGoalWithPriority.first].push_back(currGoal);
  return res;
}


void GoalStack::_refresh(const WorldState& pWorldState,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  WhatChanged whatChanged;
  _removeNoStackableGoals(whatChanged, pWorldState, pNow);
  _notifyWhatChanged(whatChanged, pNow);
}



void GoalStack::_removeNoStackableGoals(WhatChanged& pWhatChanged,
                                        const WorldState& pWorldState,
                                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  bool firstGoal = true;
  for (auto itGoalsGroup = _goals.end(); itGoalsGroup != _goals.begin(); )
  {
    --itGoalsGroup;
    for (auto itGoal = itGoalsGroup->second.begin(); itGoal != itGoalsGroup->second.end(); )
    {
      if (pWorldState.isGoalSatisfied(*itGoal))
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


void GoalStack::_notifyWhatChanged(WhatChanged& pWhatChanged,
                                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  if (pWhatChanged.goals)
    onGoalsChanged(_goals);
}


} // !cp
