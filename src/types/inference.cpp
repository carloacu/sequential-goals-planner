#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(std::unique_ptr<FactCondition> pCondition,
                     std::unique_ptr<FactModification> pFactsToModify,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : parameters(),
    condition(pCondition ? std::move(pCondition) : std::unique_ptr<FactCondition>()),
    factsToModify(pFactsToModify ? std::move(pFactsToModify) : std::unique_ptr<FactModification>()),
    goalsToAdd(pGoalsToAdd),
    isReachable(condition && condition->canBeTrue() && (!pGoalsToAdd.empty() || (factsToModify && factsToModify->canModifySomethingInTheWorld())))
{
  assert(condition);
  assert(factsToModify || !goalsToAdd.empty());
}


} // !cp
