#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATIONWITHGOAL_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATIONWITHGOAL_HPP

#include <memory>
#include <string>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/actioninvocation.hpp>


namespace cp
{

/// Struct gathering the result of one step of the planner.
struct CONTEXTUALPLANNER_API ActionInvocationWithGoal
{
  /// Construct ActionInvocationWithGoal.
  ActionInvocationWithGoal(const std::string& pActionId,
                           const std::map<Parameter, std::set<Entity>>& pParameters,
                           std::unique_ptr<cp::Goal> pFromGoal,
                           int pFromGoalPriority);

  /// Construct a copy.
  ActionInvocationWithGoal(const ActionInvocationWithGoal& pOther);

  /// Copy operator.
  void operator=(const ActionInvocationWithGoal& pOther);

  /// Action with his parameters.
  ActionInvocation actionInvocation;
  /// Goal that motivated the action.
  std::unique_ptr<cp::Goal> fromGoal;
  /// Priority of the goal that motivated the action.
  int fromGoalPriority;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ACTIONINVOCATIONWITHGOAL_HPP
