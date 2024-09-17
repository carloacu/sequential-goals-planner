#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(std::unique_ptr<Condition> pCondition,
                     std::unique_ptr<WorldStateModification> pFactsToModify,
                     const std::vector<Parameter>& pParameters,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : parameters(pParameters),
    condition(pCondition ? std::move(pCondition) : std::unique_ptr<Condition>()),
    factsToModify(pFactsToModify ? std::move(pFactsToModify) : std::unique_ptr<WorldStateModification>()),
    goalsToAdd(pGoalsToAdd)
{
  assert(condition);
  assert(factsToModify || !goalsToAdd.empty());
}


} // !cp
