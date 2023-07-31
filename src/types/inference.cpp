#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(std::unique_ptr<FactCondition> pCondition,
                     const cp::SetOfFacts& pFactsToModify,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : condition(pCondition ? std::move(pCondition) : std::unique_ptr<FactCondition>()),
    factsToModify(pFactsToModify),
    goalsToAdd(pGoalsToAdd),
    isReachable(condition && condition->canBeTrue() && (!pGoalsToAdd.empty() || pFactsToModify.canModifySomethingInTheWorld()))
{
  assert(condition);
}


} // !cp
