#ifndef INCLUDE_CONTEXTUALPLANNER_ACTION_HPP
#define INCLUDE_CONTEXTUALPLANNER_ACTION_HPP

#include <vector>
#include "api.hpp"
#include <contextualplanner/setoffacts.hpp>


namespace cp
{


struct CONTEXTUALPLANNER_API Action
{
  Action(const SetOfFacts& pPreconditions,
         const SetOfFacts& pEffects,
         const SetOfFacts& pPreferInContext = {},
         bool pShouldBeDoneAsapWithoutHistoryCheck = false)
    : parameters(),
      preconditions(pPreconditions),
      preferInContext(pPreferInContext),
      effects(pEffects),
      shouldBeDoneAsapWithoutHistoryCheck(pShouldBeDoneAsapWithoutHistoryCheck)
  {
  }

  std::vector<std::string> parameters;
  SetOfFacts preconditions;
  SetOfFacts preferInContext;
  SetOfFacts effects;
  // If this it's true it will have a very high priority for the planner.
  // It is approriate to use that for deduction actions.
  bool shouldBeDoneAsapWithoutHistoryCheck;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_ACTION_HPP
