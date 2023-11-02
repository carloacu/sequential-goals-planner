#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP

#include <set>
#include <map>
#include <memory>
#include "../util/api.hpp"
#include <contextualplanner/types/historical.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/factsalreadychecked.hpp>
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/goalstack.hpp>
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
struct CONTEXTUALPLANNER_API Problem
{
  /// Construct a problem.
  Problem() = default;
  /// Construct a problem from another problem.
  Problem(const Problem& pOther);

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
                        const std::vector<Goal>* pGoalsToAddInCurrentPriority);


  GoalStack goalStack{};
  WorldState worldState{};


  // Inferences
  // ----------

  /**
   * @brief Add a set of inferences.
   * @param pSetOfInferencesId Identifier of the set of inferences to add.
   * @param pSetOfInferences Set of inferences to add.
   *
   * If the identifier is already used, the addition will not be done.
   */
  void addSetOfInferences(const SetOfInferencesId& pSetOfInferencesId,
                          const std::shared_ptr<const SetOfInferences>& pSetOfInferences);

  /**
   * @brief Remove a set of inferences.
   * @param pSetOfInferencesId Identifier of the set of inferences to remove.
   *
   * If the inference is not found, this function will have no effect.
   * No exception will be raised.
   */
  void removeSetOfInferences(const SetOfInferencesId& pSetOfInferencesId);

  void clearInferences();

  /// Get the set of inferences.
  const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& getSetOfInferences() const { return _setOfInferences; }


  // Historical of actions done
  // --------------------------

  /// Historical of actions done.
  Historical historical{};



private:
  /// Map set of inferences identifiers to the set of inferences.
  std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>> _setOfInferences{};
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
