#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP

#include <map>
#include <memory>
#include <string>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>


namespace cp
{

/// Struct gathering the identifier of an action with some parameter values.
struct CONTEXTUALPLANNER_API ActionInstance
{
  /// Construct ActionInstance.
  ActionInstance(const std::string& pActionId,
                 const std::map<std::string, std::set<std::string>>& pParameters);


  /**
   * @brief Serialize in a string.
   * @return String containing the action identifier with his parameters.
   */
  std::string toStr() const;

  /// Identifer of the action.
  std::string actionId;
  /// Action parameter values.
  std::map<std::string, std::set<std::string>> parameters;
};


/// Struct gathering the result of one step of the planner.
struct CONTEXTUALPLANNER_API OneStepOfPlannerResult
{
  /// Construct OneStepOfPlannerResult.
  OneStepOfPlannerResult(const std::string& pActionId,
                         const std::map<std::string, std::set<std::string>>& pParameters,
                         std::unique_ptr<cp::Goal> pFromGoal,
                         int pFromGoalPriority);

  OneStepOfPlannerResult(const OneStepOfPlannerResult& pOther);

  void operator=(const OneStepOfPlannerResult& pOther);

  /// Action with his parameters.
  ActionInstance actionInstance;
  /// Goal that motivated the action.
  std::unique_ptr<cp::Goal> fromGoal;
  /// Priority of the goal that motivated the action.
  int fromGoalPriority;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ONESTEPPLANNERRESULT_HPP
