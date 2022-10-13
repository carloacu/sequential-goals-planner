#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(const cp::SetOfFacts& pCondition,
                     const cp::SetOfFacts& pFactsToModify,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : condition(pCondition),
    factsToModify(pFactsToModify),
    goalsToAdd(pGoalsToAdd)
{
  for (auto itFact = condition.facts.begin(); itFact != condition.facts.end(); )
  {
    if (itFact->isPunctual())
    {
      punctualFactsCondition.insert(*itFact);
      itFact = condition.facts.erase(itFact);
    }
    else
    {
      ++itFact;
    }
  }
}


} // !cp
