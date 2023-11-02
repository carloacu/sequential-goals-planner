#include <contextualplanner/types/problem.hpp>
#include <map>
#include <sstream>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/onestepofplannerresult.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{


Problem::Problem(const Problem& pOther)
  : historical(pOther.historical),
    goalStack(pOther.goalStack),
    worldState(pOther.worldState),
    _setOfInferences(pOther._setOfInferences)
{
}


void Problem::notifyActionDone(const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                               const std::unique_ptr<FactModification>& pEffect,
                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                               const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                               const std::vector<Goal>* pGoalsToAddInCurrentPriority)
{
  historical.notifyActionDone(pOnStepOfPlannerResult.actionInstance.actionId);

  worldState.notifyActionDone(pOnStepOfPlannerResult, pEffect, goalStack, _setOfInferences, pNow);

  goalStack.notifyActionDone(pOnStepOfPlannerResult, pEffect, pNow, pGoalsToAdd,
                              pGoalsToAddInCurrentPriority, worldState);
}


void Problem::addSetOfInferences(const SetOfInferencesId& pSetOfInferencesId,
                                 const std::shared_ptr<const SetOfInferences>& pSetOfInferences)
{
  _setOfInferences.emplace(pSetOfInferencesId, pSetOfInferences);
  worldState.clearAccessibleAndRemovableFacts();
}

void Problem::removeSetOfInferences(const SetOfInferencesId& pSetOfInferencesId)
{
  auto it = _setOfInferences.find(pSetOfInferencesId);
  if (it != _setOfInferences.end())
    _setOfInferences.erase(it);
  worldState.clearAccessibleAndRemovableFacts();
}

void Problem::clearInferences()
{
  _setOfInferences.clear();
  worldState.clearAccessibleAndRemovableFacts();
}


} // !cp
