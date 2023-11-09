#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP

#include <set>
#include <map>
#include <memory>
#include "../util/api.hpp"
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/factsalreadychecked.hpp>
#include <contextualplanner/types/inference.hpp>
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/util/observableunsafe.hpp>


namespace cp
{
struct Domain;
struct ProblemUpdate;
struct SetOfInferences;
struct OneStepOfPlannerResult;
struct GoalStack;


/// Current world, goal for the world and historical of actions done.
struct CONTEXTUALPLANNER_API WorldState
{
  /// Construct a problem.
  WorldState() = default;
  /// Construct a problem from another problem.
  WorldState(const WorldState& pOther);

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
                        GoalStack& pGoalStack,
                        const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  // World state
  // -----------

  /// Be notified when facts changed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsChanged{};
  /// Be notified about the punctual facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onPunctualFacts{};
  /// Be notified about the added facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsAdded{};
  /// Be notified about the removed facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsRemoved{};

  /**
  * @brief Add a fact.
  * @param pFact Fact to add.
  * @param pNow Current time.
  */
  bool addFact(const Fact& pFact,
               GoalStack& pGoalStack,
               const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Add several facts.
  * @param pSetOfFacts Facts to add.
  * @param pNow Current time.
  */
  template<typename FACTS>
  bool addFacts(const FACTS& pFacts,
                GoalStack& pGoalStack,
                const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Check if the world has a fact.
  bool hasFact(const Fact& pFact) const;

  /**
  * @brief Remove a fact.
  * @param pFact Fact to remove.
  * @param pNow Current time.
  */
  bool removeFact(const Fact& pFact,
                  GoalStack& pGoalStack,
                  const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Remove several facts.
  * @param pFacts Facts to remove.
  * @param pNow Current time.
  */
  template<typename FACTS>
  bool removeFacts(const FACTS& pFacts,
                   GoalStack& pGoalStack,
                   const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Modify some facts in the world.
  * @param pSetOfFacts Facts to modify in the world.
  * @param pNow Current time.
  */
  bool modifyFacts(const std::unique_ptr<FactModification>& pFactModification,
                   GoalStack& pGoalStack,
                   const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Set the facts of the world.
   * @param pFacts New set of facts for the world.
   * @param pNow Current time.
   */
  void setFacts(const std::set<Fact>& pFacts,
                GoalStack& pGoalStack,
                const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Fill accessible facts internally, according to a domain.
   * @param pDomain Domain containing the actions.
   */
  void fillAccessibleFacts(const Domain& pDomain);

  /**
   * @brief Can a fact become true, according to the world and the accessible facts stored internally in this object.
   * @param pFact Fact to check if it can become true.
   * @return True if the fact can become true, false otherwise.
   */
  bool canFactBecomeTrue(const Fact& pFact) const;

  std::string getFactValue(const Fact& pFact) const;

  void forAllInstruction(const std::string& pParameterName,
                         const Fact& pFact,
                         std::set<Fact>& pParameterValues) const;

  /// Facts of the world.
  const std::set<Fact>& facts() const { return _facts; }
  /// Fact names to facts in the world.
  const std::map<std::string, std::set<Fact>>& factNamesToFacts() const { return _factNamesToFacts; }




  // Goal stack
  // ----------

  /**
   * @brief Is an optional fact satisfied in the problem.
   * @param pFactOptional Input optional fact.
   * @return True if the input optional fact is satisfied, false otherwise.
   */
  bool isOptionalFactSatisfied(const FactOptional& pFactOptional) const;



  /**
   * @brief Check if a goal is satisfied.<br/>
   * A goal is not satisfied if the condition is true (if it exist) and if the value of the goal is not true.
   * @param pGoal Goal to check.
   * @return True if the goal is satisfied.
   */
  bool isGoalSatisfied(const Goal& pGoal) const;

  bool isFactPatternSatisfied(const FactOptional& pFactOptional,
                              const std::set<Fact>& pPunctualFacts,
                              const std::set<Fact>& pRemovedFacts,
                              std::map<std::string, std::set<std::string>>* pParametersPtr,
                              bool* pCanBecomeTruePtr) const;

  /// Clear accessible and removable facts.
  void clearAccessibleAndRemovableFacts();



private:
  /// Facts of the world.
  std::set<Fact> _facts{};
  /// Fact names to facts in the world.
  std::map<std::string, std::set<Fact>> _factNamesToFacts{};
  /// Facts that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFacts{};
  /// Facts with values that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFactsWithAnyValues{};
  /// Facts that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFacts{};
  /// Know if we need to add accessible facts.
  bool _needToAddAccessibleFacts = true;

  /// Stored what changed.
  struct WhatChanged
  {
    /// Punctual facts that are pinged.
    std::set<Fact> punctualFacts;
    /// Facts that we added in the world.
    std::set<Fact> addedFacts;
    /// Facts that we removed in the world.
    std::set<Fact> removedFacts;

    /// Check if something changed.
    bool somethingChanged() const { return !punctualFacts.empty() || !addedFacts.empty() || !removedFacts.empty(); }
    /// Has some facts to add or to remove.
    bool hasFactsModifications() const { return !addedFacts.empty() || !removedFacts.empty(); }
  };


  /**
   * @brief Add facts without raising a notification.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pFacts Facts to add.
   * @param[in] pNow Current time.
   * @return True if some facts were added, false otherwise.
   */
  template<typename FACTS>
  void _addFacts(WhatChanged& pWhatChanged,
                 const FACTS& pFacts,
                 GoalStack& pGoalStack,
                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove facts without raising a notification.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pFacts Facts to add.
   * @param[in] pNow Current time.
   * @return True if some facts were removed, false otherwise.
   */
  template<typename FACTS>
  void _removeFacts(WhatChanged& pWhatChanged,
                    const FACTS& pFacts,
                    GoalStack& pGoalStack,
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Feed accessible facts from a set of actions.
   * @param pActions Set of actions.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                            const Domain& pDomain,
                                            FactsAlreadyChecked& pFactsAlreadychecked,
                                            const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences);

  /**
   * @brief Feed accessible facts from a set of inferences.
   * @param pInferences Set of inferences.
   * @param pAllInferences Set of all the possible inferences.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                               const std::map<InferenceId, Inference>& pAllInferences,
                                               const Domain& pDomain,
                                               FactsAlreadyChecked& pFactsAlreadychecked,
                                               const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences);

  /**
   * @brief Feed accessible facts from a condition and an effect.
   * @param pCondition condition to check.
   * @param pEffect Effect to aply.
   * @param pParameters Parameter names.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromDeduction(const std::unique_ptr<Condition>& pCondition,
                                         const ProblemUpdate& pEffect,
                                         const std::vector<std::string>& pParameters,
                                         const Domain& pDomain,
                                         FactsAlreadyChecked& pFactsAlreadychecked,
                                         const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences);

  /**
   * @brief Feed accessible facts from a fact.
   * @param pFact A fact.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromFact(const Fact& pFact,
                                    const Domain& pDomain,
                                    FactsAlreadyChecked& pFactsAlreadychecked,
                                    const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences);

  /**
   * @brief Feed accessible facts from a negated fact.
   * @param pFact A negated fact.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromNotFact(const Fact& pFact,
                                       const Domain& pDomain,
                                       FactsAlreadyChecked& pFactsAlreadychecked,
                                       const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences);

  /**
   * @brief Modify some facts in the world.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pSetOfFacts Facts to modify in the world.
   * @param[in] pNow Current time.
   */
  void _modifyFacts(WhatChanged& pWhatChanged,
                    const std::unique_ptr<FactModification>& pFactModification,
                    GoalStack& pGoalStack,
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  /**
   * @brief Try to apply some inference according to the fact that changed.
   * @param[in, out] pInferencesAlreadyApplied Inferences that we already considered.
   * @param[in, out] pWhatChanged The facts that changed.
   * @param[in] pInferenceIds Inferences to consider.
   * @param[in] pInferences All the inferences.
   * @param[in] pNow Current time.
   * @return True if at least one inference was applied.
   */
  bool _tryToApplyInferences(std::set<InferenceId>& pInferencesAlreadyApplied,
                             WhatChanged& pWhatChanged,
                             GoalStack& pGoalStack,
                             const std::set<InferenceId>& pInferenceIds,
                             const std::map<InferenceId, Inference>& pInferences,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Do inferences and raise the observables if some facts or goals changed.
   * @param[in] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _notifyWhatChanged(WhatChanged& pWhatChanged,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& pSetOfInferences,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  friend struct ConditionNode;
  friend struct ConditionFact;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP
