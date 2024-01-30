#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP

#include <memory>
#include <string>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/actioninvocation.hpp>


namespace cp
{

/// Struct gathering the result of one step of the planner.
struct CONTEXTUALPLANNER_API OneStepOfPlannerResult
{
  /// Construct OneStepOfPlannerResult.
  OneStepOfPlannerResult(const std::string& pActionId,
                         const std::map<std::string, std::set<std::string>>& pParameters,
                         std::unique_ptr<cp::Goal> pFromGoal,
                         int pFromGoalPriority);

  /// Construct a copy.
  OneStepOfPlannerResult(const OneStepOfPlannerResult& pOther);

  /// Copy operator.
  void operator=(const OneStepOfPlannerResult& pOther);

  /// Action with his parameters.
  ActionInvocation actionInvocation;
  /// Goal that motivated the action.
  std::unique_ptr<cp::Goal> fromGoal;
  /// Priority of the goal that motivated the action.
  int fromGoalPriority;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP
