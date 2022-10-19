#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(const cp::SetOfFacts& pCondition,
                     const cp::SetOfFacts& pFactsToModify,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : _condition(pCondition),
    _factsToModify(pFactsToModify),
    _goalsToAdd(pGoalsToAdd),
    _isReachable(pCondition.canBeTrue() && (!pGoalsToAdd.empty() || pFactsToModify.canModifySomethingInTheWorld()))
{
}


} // !cp
