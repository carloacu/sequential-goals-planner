#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(const cp::SetOfFacts& pCondition,
                     const cp::SetOfFacts& pFactsToModify,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : _condition(pCondition),
    _punctualFactsCondition(),
    _factsToModify(pFactsToModify),
    _goalsToAdd(pGoalsToAdd)
{
  for (auto itFact = _condition.facts.begin(); itFact != _condition.facts.end(); )
  {
    if (itFact->isPunctual())
    {
      _punctualFactsCondition.insert(*itFact);
      itFact = _condition.facts.erase(itFact);
    }
    else
    {
      ++itFact;
    }
  }
}


} // !cp
