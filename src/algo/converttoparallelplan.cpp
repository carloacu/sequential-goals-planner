#include "converttoparallelplan.hpp"
#include <list>
#include <set>
#include <prioritizedgoalsplanner/types/action.hpp>
#include <prioritizedgoalsplanner/types/actioninvocationwithgoal.hpp>
#include <prioritizedgoalsplanner/types/actionstodoinparallel.hpp>
#include <prioritizedgoalsplanner/types/domain.hpp>
#include <prioritizedgoalsplanner/types/lookforanactionoutputinfos.hpp>
#include <prioritizedgoalsplanner/types/problem.hpp>
#include <prioritizedgoalsplanner/types/worldstate.hpp>
#include <prioritizedgoalsplanner/prioritizedgoalsplanner.hpp>
#include "notifyactiondone.hpp"

namespace pgp
{

namespace
{

struct ActionDataForParallelisation
{
  ActionDataForParallelisation(const Action& pAction, ActionInvocationWithGoal&& pActionInvWithGoal)
    : action(pAction),
      actionInvWithGoal(std::move(pActionInvWithGoal)),
      conditionWithParameterFilled()
  {
  }

  const Condition* getConditionWithoutParameterPtr()
  {
    if (!action.precondition)
      return nullptr;
    if (actionInvWithGoal.actionInvocation.parameters.empty())
      return &*action.precondition;
    if (!conditionWithParameterFilled)
      conditionWithParameterFilled = action.precondition->clone(&actionInvWithGoal.actionInvocation.parameters);
    return &*conditionWithParameterFilled;
  }

  const WorldStateModification* getWorldStateModificationAtStartWithoutParameterPtr()
  {
    if (!action.effect.worldStateModificationAtStart)
      return nullptr;
    if (actionInvWithGoal.actionInvocation.parameters.empty())
      return &*action.effect.worldStateModificationAtStart;
    if (!worldStateModificationAtStartWithParameterFilled)
      worldStateModificationAtStartWithParameterFilled = action.effect.worldStateModificationAtStart->clone(&actionInvWithGoal.actionInvocation.parameters);
    return &*worldStateModificationAtStartWithParameterFilled;
  }

  const WorldStateModification* getWorldStateModificationWithoutParameterPtr()
  {
    if (!action.effect.worldStateModification)
      return nullptr;
    if (actionInvWithGoal.actionInvocation.parameters.empty())
      return &*action.effect.worldStateModification;
    if (!worldStateModificationWithParameterFilled)
      worldStateModificationWithParameterFilled = action.effect.worldStateModification->clone(&actionInvWithGoal.actionInvocation.parameters);
    return &*worldStateModificationWithParameterFilled;
  }

  const WorldStateModification* getPotentialWorldStateModificationWithoutParameterPtr()
  {
    if (!action.effect.potentialWorldStateModification)
      return nullptr;
    if (actionInvWithGoal.actionInvocation.parameters.empty())
      return &*action.effect.potentialWorldStateModification;
    if (!potentialWorldStateModificationWithParameterFilled)
      potentialWorldStateModificationWithParameterFilled = action.effect.potentialWorldStateModification->clone(&actionInvWithGoal.actionInvocation.parameters);
    return &*potentialWorldStateModificationWithParameterFilled;
  }

  const std::set<FactOptional>& getAllOptFactsThatCanBeModified()
  {
    if (!factsThatCanBeModifiedPtr)
    {
      factsThatCanBeModifiedPtr = std::make_unique<std::set<FactOptional>>();
      auto addWmToRes = [&](const WorldStateModification* pWmPtr)
      {
        if (pWmPtr)
        {
          pWmPtr->forAllThatCanBeModified([&](const FactOptional& pFactOptional) {
              factsThatCanBeModifiedPtr->insert(pFactOptional);
              return ContinueOrBreak::CONTINUE;
          });
        }
      };
      addWmToRes(getWorldStateModificationAtStartWithoutParameterPtr());
      addWmToRes(getWorldStateModificationWithoutParameterPtr());
      addWmToRes(getPotentialWorldStateModificationWithoutParameterPtr());
    }
    return *factsThatCanBeModifiedPtr;
  }

  bool canBeInParallel(ActionDataForParallelisation& pOther)
  {
    const auto& effectFacts = getAllOptFactsThatCanBeModified();
    auto* otherConditionPtr = pOther.getConditionWithoutParameterPtr();
    if (otherConditionPtr != nullptr && otherConditionPtr->hasAContradictionWith(effectFacts))
      return false;

    const auto& otherEffectFacts = pOther.getAllOptFactsThatCanBeModified();
    auto* conditionPtr = getConditionWithoutParameterPtr();
    if (conditionPtr != nullptr && conditionPtr->hasAContradictionWith(otherEffectFacts))
      return false;

    return true;
  }

  bool canBeInParallelOfList(std::list<ActionDataForParallelisation>& pOthers)
  {
    for (auto& currOther : pOthers)
      if (!canBeInParallel(currOther))
        return false;
    return true;
  }

  const Action& action;
  ActionInvocationWithGoal actionInvWithGoal;
  std::unique_ptr<Condition> conditionWithParameterFilled;
  std::unique_ptr<WorldStateModification> worldStateModificationAtStartWithParameterFilled;
  std::unique_ptr<WorldStateModification> worldStateModificationWithParameterFilled;
  std::unique_ptr<WorldStateModification> potentialWorldStateModificationWithParameterFilled;
  std::unique_ptr<std::set<FactOptional>> factsThatCanBeModifiedPtr;
};


std::list<pgp::Goal> _checkSatisfiedGoals(
    Problem& pProblem,
    const Domain& pDomain,
    std::list<std::list<ActionDataForParallelisation>>::iterator pCurrItInPlan,
    std::list<std::list<ActionDataForParallelisation>>& pPlan,
    const ActionDataForParallelisation* pActionToSkipPtr,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  std::list<pgp::Goal> res;
  auto& setOfEvents = pDomain.getSetOfEvents();
  const auto& ontology = pDomain.getOntology();

  LookForAnActionOutputInfos lookForAnActionOutputInfos;

  ++pCurrItInPlan;
  while (pCurrItInPlan != pPlan.end())
  {
    std::list<ActionDataForParallelisation*> actionsInParallel;
    for (auto& currAction : *pCurrItInPlan)
    {
      if (&currAction == pActionToSkipPtr)
        continue;

      const auto* conditionPtr = currAction.getConditionWithoutParameterPtr();
      if (conditionPtr != nullptr && !conditionPtr->isTrue(pProblem.worldState))
        return {};

      auto* worldStateModificationAtStartWithoutParameterPtr = currAction.getWorldStateModificationAtStartWithoutParameterPtr();
      if (worldStateModificationAtStartWithoutParameterPtr != nullptr)
        pProblem.worldState.modify(worldStateModificationAtStartWithoutParameterPtr, pProblem.goalStack, setOfEvents, ontology, pProblem.entities, pNow);
      actionsInParallel.emplace_back(&currAction);

      const auto* worldStateModificationWithoutParameterPtr = currAction.getWorldStateModificationWithoutParameterPtr();
      if (worldStateModificationWithoutParameterPtr != nullptr)
        pProblem.worldState.modify(worldStateModificationWithoutParameterPtr, pProblem.goalStack, setOfEvents, ontology, pProblem.entities, pNow);

      const auto* potentialWorldStateModificationWithoutParameterPtr = currAction.getPotentialWorldStateModificationWithoutParameterPtr();
      if (potentialWorldStateModificationWithoutParameterPtr != nullptr)
        pProblem.worldState.modify(potentialWorldStateModificationWithoutParameterPtr, pProblem.goalStack, setOfEvents, ontology, pProblem.entities, pNow);

      pProblem.goalStack.notifyActionDone(currAction.actionInvWithGoal, pNow,
                                          &currAction.action.effect.goalsToAdd,
                                          &currAction.action.effect.goalsToAddInCurrentPriority, pProblem.worldState, &lookForAnActionOutputInfos);
    }
    ++pCurrItInPlan;
  }

  lookForAnActionOutputInfos.moveGoalsDone(res);
  return res;
}

void _notifyActionsDoneAndRemoveCorrespondingGoals(std::list<Goal>& pGoals,
                                                   const std::list<ActionDataForParallelisation>& pActions,
                                                   Problem& pProblem,
                                                   const Domain& pDomain,
                                                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  LookForAnActionOutputInfos lookForAnActionOutputInfos;
  for (auto& currActionTmpData : pActions)
    notifyActionDone(pProblem, pDomain, currActionTmpData.actionInvWithGoal, pNow, &lookForAnActionOutputInfos);
  std::list<Goal> goalsSatisfied;
  lookForAnActionOutputInfos.moveGoalsDone(goalsSatisfied);
  for (auto& currGoal : goalsSatisfied)
    pGoals.remove(currGoal);
}


}



std::list<ActionsToDoInParallel> toParallelPlan
(std::list<ActionInvocationWithGoal>& pSequentialPlan,
 bool pParalleliseOnyFirstStep,
 Problem& pProblem,
 const Domain& pDomain,
 std::list<Goal>& pGoals,
 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  const auto& actions = pDomain.actions();
  std::list<std::list<ActionDataForParallelisation>> currentRes;

  // Connvert list of actions to a list of list of actions
  while (!pSequentialPlan.empty())
  {
    ActionInvocationWithGoal& actionInvWithGoal = pSequentialPlan.front();
    auto itAction = actions.find(actionInvWithGoal.actionInvocation.actionId);
    if (itAction == actions.end())
      throw std::runtime_error("ActionId \"" + actionInvWithGoal.actionInvocation.actionId + "\" not found in algorithm to manaage parralelisation");
    std::list<ActionDataForParallelisation> actionInASubList;
    actionInASubList.emplace_back(itAction->second, std::move(actionInvWithGoal));
    currentRes.emplace_back(std::move(actionInASubList));
    pSequentialPlan.pop_front();
  }

  for (auto itPlanStep = currentRes.begin(); itPlanStep != currentRes.end(); ++itPlanStep)
  {
    for (ActionDataForParallelisation& currActionTmpData : *itPlanStep)
      notifyActionStarted(pProblem, pDomain, currActionTmpData.actionInvWithGoal, pNow);

    auto itPlanStepCandidate = itPlanStep;
    ++itPlanStepCandidate;
    while (itPlanStepCandidate != currentRes.end())
    {
      if (!itPlanStepCandidate->empty())
      {
        auto& actionInvocationCand = itPlanStepCandidate->front();
        if (actionInvocationCand.canBeInParallelOfList(*itPlanStep))
        {
          const Condition* conditionWithoutParameterPtr = actionInvocationCand.getConditionWithoutParameterPtr();
          if (conditionWithoutParameterPtr == nullptr ||
              conditionWithoutParameterPtr->isTrue(pProblem.worldState))
          {
            auto tmpProblem = pProblem;
            notifyActionStarted(tmpProblem, pDomain, actionInvocationCand.actionInvWithGoal, pNow);
            notifyActionDone(tmpProblem, pDomain, actionInvocationCand.actionInvWithGoal, pNow);
            auto remainingGoals = pGoals;
            _notifyActionsDoneAndRemoveCorrespondingGoals(remainingGoals, *itPlanStep, tmpProblem, pDomain, pNow);

            auto goalsCand = _checkSatisfiedGoals(tmpProblem, pDomain, itPlanStep, currentRes, &actionInvocationCand, pNow);
            if (goalsCand == remainingGoals)
            {
              notifyActionStarted(pProblem, pDomain, actionInvocationCand.actionInvWithGoal, pNow);
              itPlanStep->emplace_back(std::move(actionInvocationCand));
              itPlanStepCandidate = currentRes.erase(itPlanStepCandidate);
              continue;
            }
          }

        }
      }
      ++itPlanStepCandidate;
    }

    if (pParalleliseOnyFirstStep)
      break;
    _notifyActionsDoneAndRemoveCorrespondingGoals(pGoals, *itPlanStep, pProblem, pDomain, pNow);
  }

  std::list<ActionsToDoInParallel> res;
  for (auto& currResStep : currentRes)
  {
    ActionsToDoInParallel subRes;
    for (auto& currActionTmpData : currResStep)
      subRes.actions.emplace_back(std::move(currActionTmpData.actionInvWithGoal));
    if (!subRes.actions.empty())
      res.emplace_back(std::move(subRes));
  }
  return res;
}



} // End of namespace pgp
