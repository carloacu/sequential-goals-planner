#ifndef INCLUDE_CONTEXTUALPLANNER_GOALSTACK_HPP
#define INCLUDE_CONTEXTUALPLANNER_GOALSTACK_HPP

#include <set>
#include <map>
#include <memory>
#include "../util/api.hpp"
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/factsalreadychecked.hpp>
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/inference.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/util/observableunsafe.hpp>


namespace cp
{
struct Domain;
struct ProblemUpdate;
struct SetOfInferences;
struct OneStepOfPlannerResult;


/// Current world, goal for the world and historical of actions done.
struct CONTEXTUALPLANNER_API GoalStack
{
  /// Construct a problem.
  GoalStack() = default;
  /// Construct a problem from another problem.
  GoalStack(const GoalStack& pOther);

  /**
   * @brief Notify that an action has been done.
   * @param[in] pOnStepOfPlannerResult Planner result step that motivated this action.
   * @param pEffect Effect of the action done.
   * @param pNow Current time.
   * @param pGoalsToAdd Priorities to goals to add.
   * @param pGoalsToAddInCurrentPriority Goals to add in current priority.
   */
  void notifyActionDone(const OneStepOfPlannerResult& pOneStepOfPlannerResult,
                        const std::unique_ptr<FactModification>& pEffect,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                        const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                        const std::vector<Goal>* pGoalsToAddInCurrentPriority,
                        const WorldState& pWorldState);

  /// Be notified when goals changed.
  cpstd::observable::ObservableUnsafe<void (const std::map<int, std::vector<Goal>>&)> onGoalsChanged{};


  // Goal stack
  // ----------

  /// Get the goal serialized in string in top of the stack of goals, an empty string is returned if there is no goal.
  std::string getCurrentGoalStr() const;

  /// Get the goal in top of the stack of goals, nullptr is returned if there is no goal.
  const Goal* getCurrentGoalPtr() const;

  /**
   * @brief Iterate on goals and remove non persistent goals.
   * @param pManageGoal Callback to manage the goal. If the callback returns true we stop the iteration.
   * @param pNow Current time.
   */
  void iterateOnGoalsAndRemoveNonPersistent(const std::function<bool (Goal&, int)>& pManageGoal,
                                            const WorldState& pWorldState,
                                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Default priority.
  static const int defaultPriority;

  /**
   * @brief Set the goals.
   * @param pGoals Map of priority to the associated goals.
   * @param pNow Current time.
   */
  void setGoals(const std::map<int, std::vector<Goal>>& pGoals,
                const WorldState& pWorldState,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Set the goals.
   * @param pGoals Set of goals.
   * @param pNow Current time.
   * @param pPriority Priority of the goals.
   */
  void setGoals(const std::vector<Goal>& pGoals,
                const WorldState& pWorldState,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                int pPriority = defaultPriority);

  /**
   * @brief Add some goals.
   * @param pGoals Map of priority to the associated goals.
   * @param pNow Current time.
   */
  void addGoals(const std::map<int, std::vector<Goal>>& pGoals,
                const WorldState& pWorldState,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Add some goals.
   * @param pGoals Set of goals to add.
   * @param pNow Current time.
   * @param pPriority Priority of the goals to add.
   */
  void addGoals(const std::vector<Goal>& pGoals,
                const WorldState& pWorldState,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                int pPriority = defaultPriority);

  /**
   * @brief Add a goal in front of the existing goals that have the same level of priority.
   * @param pGoal Goal to add.
   * @param pNow Current time.
   * @param pPriorityPriority of the goal to add.
   */
  void pushFrontGoal(const Goal& pGoal,
                     const WorldState &pWorldState,
                     const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                     int pPriority = defaultPriority);

  /**
   * @brief Add a goal on bottom of the existing goals that have the same level of priority.
   * @param pGoal Goal to add.
   * @param pNow Current time.
   * @param pPriorityPriority of the goal to add.
   */
  void pushBackGoal(const Goal& pGoal,
                    const WorldState& pWorldState,
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                    int pPriority = defaultPriority);

  /**
   * @brief Change the priority of a goal.
   * @param pGoalStr Goal concerned.
   * @param pPriority New priority to set.
   * @param pPushFrontOrBottomInCaseOfConflictWithAnotherGoal Push in front or in bottom in case of conflict with another goal.
   * @param pNow Current time.
   */
  void changeGoalPriority(const std::string& pGoalStr,
                          int pPriority,
                          bool pPushFrontOrBottomInCaseOfConflictWithAnotherGoal,
                          const WorldState &pWorldState,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Clear the goals.
   * @param pNow Current time.
   */
  void clearGoals(const WorldState& pWorldState,
                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove some goals.
   * @param pGoalGroupId Group identifier of the goals to remove.
   * @param pNow Current time.
   * @return True if at least one goal is removed, false otherwise.
   */
  bool removeGoals(const std::string& pGoalGroupId,
                   const WorldState& pWorldState,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove the first goals that are already satisfied.
   * @param[in] pNow Current time.
   */
  void removeFirstGoalsThatAreAlreadySatisfied(const WorldState& pWorldState,
                                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Goals to satisfy.
  const std::map<int, std::vector<Goal>>& goals() const { return _goals; }

  /**
   * @brief Get the not satisfied goals.<br/>
   * A goal is not satisfied if the condition is true (if it exist) and if the value of the goal is not true.
   * @return Map of priority to not satisfied goals.
   */
  std::map<int, std::vector<Goal>> getNotSatisfiedGoals(const WorldState &pWorldState) const;


private:
  /// Map of priority to goals.
  std::map<int, std::vector<Goal>> _goals{};
  /// Current active goal.
  const Goal* _currentGoalPtr = nullptr;

  /// Stored what changed.
  struct WhatChanged
  {
    /// True if the goals changed.
    bool goals = false;

    /// Check if something changed.
    bool somethingChanged() const { return goals; }
  };

  void _refresh(const WorldState& pWorldState,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  /**
   * @brief Remove the first goals that are already satisfied.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _removeFirstGoalsThatAreAlreadySatisfied(WhatChanged& pWhatChanged,
                                                const WorldState& pWorldState,
                                                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Iterate on goals and remove non persistent goals.
   * @param[out] pWhatChanged Get what changed.
   * @param pManageGoal Callback to manage the goal. If the callback returns true we stop the iteration.
   * @param[in] pNow Current time.
   */
  void _iterateOnGoalsAndRemoveNonPersistent(WhatChanged& pWhatChanged,
                                             const std::function<bool (Goal&, int)>& pManageGoal,
                                             const WorldState& pWorldState,
                                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Get the priority of the goal in top of the stack.
  int _getCurrentPriority(const WorldState& pWorldState) const;


  /**
   * @brief Remove no stackable goals.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _removeNoStackableGoals(WhatChanged& pWhatChanged,
                               const WorldState& pWorldState,
                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  /**
   * @brief Add some goals.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pGoals Set of goals to add.
   * @param[in] pNow Current time.
   */
  void _addGoals(WhatChanged& pWhatChanged,
                 const std::map<int, std::vector<Goal>>& pGoals,
                 const WorldState& pWorldState,
                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Do inferences and raise the observables if some facts or goals changed.
   * @param[in] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _notifyWhatChanged(WhatChanged& pWhatChanged,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  friend struct WorldState;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_GOALSTACK_HPP
