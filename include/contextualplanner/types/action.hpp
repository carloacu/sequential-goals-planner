#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP

#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/worldmodification.hpp>
#include <contextualplanner/types/setoffacts.hpp>


namespace cp
{


/// Axiomatic thing that the bot can do
struct CONTEXTUALPLANNER_API Action
{
  /**
   * @brief Construct an Action.
   * @param pPreconditions Set of facts that should be present in the world to be able to do this action.
   * @param pEffect How the world will change when this action will be finished.
   * @param pPreferInContext Set of facts that will increase the priority of this action, if they are present in the world.
   * @param pShouldBeDoneAsapWithoutHistoryCheck If true this action will done as soon as it contribute to an existing goal.
   */
  Action(const SetOfFacts& pPreconditions,
         const WorldModification& pEffect,
         const SetOfFacts& pPreferInContext = {},
         bool pShouldBeDoneAsapWithoutHistoryCheck = false)
    : parameters(),
      preconditions(pPreconditions),
      preferInContext(pPreferInContext),
      effect(pEffect),
      shouldBeDoneAsapWithoutHistoryCheck(pShouldBeDoneAsapWithoutHistoryCheck)
  {
  }

  /// Parameter names of this action.
  std::vector<std::string> parameters;
  /// Set of facts that should be present in the world to be able to do this action.
  SetOfFacts preconditions;
  /// Set of facts that will increase the priority of this action, if they are present in the world.
  SetOfFacts preferInContext;
  /// How the world will change when this action will be finished.
  WorldModification effect;

  /**
   * If this it's true it will have a very high priority for the planner.
   * It is approriate to use that for deduction actions.
   */
  bool shouldBeDoneAsapWithoutHistoryCheck;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ACTION_HPP
