#include "notifyactiondone.hpp"
#include <contextualplanner/types/action.hpp>
#include <contextualplanner/types/actioninvocationwithgoal.hpp>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/types/worldstate.hpp>

namespace cp
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
  auto itAction = pDomain.actions().find(pOneStepOfPlannerResult.actionInvocation.actionId);
  if (itAction != pDomain.actions().end())
  {
    if (pGlobalHistorical != nullptr)
      pGlobalHistorical->notifyActionDone(pOneStepOfPlannerResult.actionInvocation.actionId);
    auto& setOfEvents = pDomain.getSetOfEvents();

    const auto& ontology = pDomain.getOntology();
    pProblem.worldState.modify(itAction->second.effect.worldStateModificationAtStart, pProblem.goalStack, setOfEvents,
                               ontology, pProblem.entities, pNow);

    notifyActionInvocationDone(pProblem, pGoalChanged, setOfEvents, pOneStepOfPlannerResult, itAction->second.effect.worldStateModification,
                               ontology, pNow,
                               &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority,
                               pLookForAnActionOutputInfosPtr);

    if (itAction->second.effect.potentialWorldStateModification)
    {
      auto potentialEffect = itAction->second.effect.potentialWorldStateModification->cloneParamSet(pOneStepOfPlannerResult.actionInvocation.parameters);
      pProblem.worldState.modify(potentialEffect, pProblem.goalStack, setOfEvents,
                                 ontology, pProblem.entities, pNow);
    }
  }
}


} // End of namespace cp
