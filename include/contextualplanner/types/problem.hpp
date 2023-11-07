#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP

#include <contextualplanner/types/goalstack.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/types/historical.hpp>
#include "../util/api.hpp"


namespace cp
{


/// Current world, goal for the world and historical of actions done.
struct CONTEXTUALPLANNER_API Problem
{
  GoalStack goalStack;
  WorldState worldState;

  /// Historical of actions done.
  Historical historical;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
