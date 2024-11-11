#include "notifyactiondone.hpp"
#include <prioritizedgoalsplanner/types/action.hpp>
#include <prioritizedgoalsplanner/types/actioninvocationwithgoal.hpp>
#include <prioritizedgoalsplanner/types/domain.hpp>
#include <prioritizedgoalsplanner/types/problem.hpp>
#include <prioritizedgoalsplanner/types/worldstate.hpp>

namespace pgp
{

void notifyActionInvocationDone(Problem& pProblem,
                                bool& pGoalChanged,
                                const std::map<SetOfEventsId, SetOfEvents>& pSetOfEvents,
                                const ActionInvocationWithGoal& pOnStepOfPlannerResult,
                                const std::unique_ptr<WorldStateModification>& pEffect,
                                const Ontology& pOntology,
                                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                                const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                                const std::vector<Goal>* pGoalsToAddInCurrentPriority,
                                LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  pProblem.historical.notifyActionDone(pOnStepOfPlannerResult.actionInvocation.actionId);

  pProblem.worldState.notifyActionDone(pOnStepOfPlannerResult, pEffect, pGoalChanged, pProblem.goalStack,
                                       pSetOfEvents, pOntology, pProblem.entities, pNow);

  pGoalChanged = pProblem.goalStack.notifyActionDone(pOnStepOfPlannerResult, pNow, pGoalsToAdd,
                                                     pGoalsToAddInCurrentPriority, pProblem.worldState, pLookForAnActionOutputInfosPtr) || pGoalChanged;
}



void updateProblemForNextPotentialPlannerResult(
    Problem& pProblem,
    bool& pGoalChanged,
    const ActionInvocationWithGoal& pOneStepOfPlannerResult,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  auto* actionPtr = pDomain.getActionPtr(pOneStepOfPlannerResult.actionInvocation.actionId);
  if (actionPtr != nullptr)
    updateProblemForNextPotentialPlannerResultWithAction(pProblem, pGoalChanged,
                                                         pOneStepOfPlannerResult, *actionPtr,
                                                         pDomain, pNow, pGlobalHistorical,
                                                         pLookForAnActionOutputInfosPtr);
}


void updateProblemForNextPotentialPlannerResultWithAction(
    Problem& pProblem,
    bool& pGoalChanged,
    const ActionInvocationWithGoal& pOneStepOfPlannerResult,
    const Action& pOneStepAction,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr)
{
  if (pGlobalHistorical != nullptr)
    pGlobalHistorical->notifyActionDone(pOneStepOfPlannerResult.actionInvocation.actionId);
  auto& setOfEvents = pDomain.getSetOfEvents();

  const auto& ontology = pDomain.getOntology();
  pProblem.worldState.modify(pOneStepAction.effect.worldStateModificationAtStart, pProblem.goalStack, setOfEvents,
                             ontology, pProblem.entities, pNow);

  notifyActionInvocationDone(pProblem, pGoalChanged, setOfEvents, pOneStepOfPlannerResult, pOneStepAction.effect.worldStateModification,
                             ontology, pNow,
                             &pOneStepAction.effect.goalsToAdd, &pOneStepAction.effect.goalsToAddInCurrentPriority,
                             pLookForAnActionOutputInfosPtr);

  if (pOneStepAction.effect.potentialWorldStateModification)
  {
    auto potentialEffect = pOneStepAction.effect.potentialWorldStateModification->cloneParamSet(pOneStepOfPlannerResult.actionInvocation.parameters);
    pProblem.worldState.modify(potentialEffect, pProblem.goalStack, setOfEvents,
                               ontology, pProblem.entities, pNow);
  }
}


} // End of namespace pgp
