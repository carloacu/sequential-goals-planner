#include "converttoparallelplan.hpp"
#include <list>
#include <contextualplanner/types/action.hpp>
#include <contextualplanner/types/actioninvocationwithgoal.hpp>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/lookforanactionoutputinfos.hpp>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/contextualplanner.hpp>
#include "notifyactiondone.hpp"

namespace cp
{

namespace
{

std::list<cp::Goal> _checkSatisfiedGoals(
    const Problem& pProblem,
    const Domain& pDomain,
    const std::list<std::list<cp::ActionInvocationWithGoal>>& pPlan,
    const cp::ActionInvocationWithGoal* pActionToDoFirstPtr,
    const cp::ActionInvocationWithGoal* pActionToSkipPtr,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  std::list<cp::Goal> res;
  auto problem = pProblem;
  const auto& actions = pDomain.actions();

  LookForAnActionOutputInfos lookForAnActionOutputInfos;
  if (pActionToDoFirstPtr != nullptr)
  {
    bool goalChanged = false;
    updateProblemForNextPotentialPlannerResult(problem, goalChanged, *pActionToDoFirstPtr, pDomain, pNow, nullptr,
                                               &lookForAnActionOutputInfos);
  }

  for (const auto& currActionsToDoInParallel : pPlan)
  {
    for (const auto& currAction : currActionsToDoInParallel)
    {
      if (&currAction == pActionToSkipPtr)
        continue;

      auto itAction = actions.find(currAction.actionInvocation.actionId);
      if (itAction != actions.end() &&
          (!itAction->second.precondition ||
           itAction->second.precondition->isTrue(problem.worldState)))
      {
        bool goalChanged = false;
        updateProblemForNextPotentialPlannerResult(problem, goalChanged, currAction, pDomain, pNow, nullptr,
                                                    &lookForAnActionOutputInfos);
      }
      else
      {
        return {};
      }
    }
  }

  lookForAnActionOutputInfos.moveGoalsDone(res);
  return res;
}


}



std::list<std::list<cp::ActionInvocationWithGoal>> toParallelPlan
(std::list<cp::ActionInvocationWithGoal>& pSequentialPlan,
 bool pParalleliseOnyFirstStep,
 const Problem& pProblem,
 const Domain& pDomain,
 const std::list<cp::Goal>& pGoals,
 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  std::list<std::list<cp::ActionInvocationWithGoal>> res;
  auto currentProblem = pProblem;

  // Connvert list of actions to a list of list of actions
  while (!pSequentialPlan.empty())
  {
    res.emplace_back(1, std::move(pSequentialPlan.front()));
    pSequentialPlan.pop_front();
  }

  const auto& actions = pDomain.actions();
  for (auto itPlanStep = res.begin(); itPlanStep != res.end(); ++itPlanStep)
  {
    for (auto& currAction : *itPlanStep)
      notifyActionStarted(currentProblem, pDomain, currAction, pNow);

    auto itPlanStepCandidate = itPlanStep;
    ++itPlanStepCandidate;
    while (itPlanStepCandidate != res.end())
    {
      if (!itPlanStepCandidate->empty())
      {
        auto& actionInvocationCand = itPlanStepCandidate->front();
        auto itActionCand = actions.find(actionInvocationCand.actionInvocation.actionId);
        if (itActionCand != actions.end() &&
            (!itActionCand->second.precondition ||
             itActionCand->second.precondition->isTrue(currentProblem.worldState)))
        {
          auto goalsCand = _checkSatisfiedGoals(currentProblem, pDomain, res, &actionInvocationCand, &actionInvocationCand, pNow);
          if (goalsCand == pGoals)
          {
            itPlanStep->emplace_back(std::move(actionInvocationCand));
            itPlanStepCandidate = res.erase(itPlanStepCandidate);
            break;
          }
        }
      }
      ++itPlanStepCandidate;
    }

    if (pParalleliseOnyFirstStep)
      break;
  }

  return res;
}



} // End of namespace cp
