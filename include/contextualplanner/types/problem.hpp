#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP

#include <set>
#include <map>
#include <memory>
#include "../util/api.hpp"
#include <contextualplanner/types/historical.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/inference.hpp>
#include <contextualplanner/types/setoffacts.hpp>
#include <contextualplanner/util/observableunsafe.hpp>


namespace cp
{
struct Domain;
struct WorldModification;
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
   * @param pGoalsToAdd Goals to add.
   */
  void notifyActionDone(const OneStepOfPlannerResult& pOneStepOfPlannerResult,
                        const SetOfFacts& pEffect,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                        const std::map<int, std::vector<Goal>>* pGoalsToAdd);


  // World state
  // -----------

  /// Be notified when a variable to value changed.
  cpstd::observable::ObservableUnsafe<void (const std::map<std::string, std::string>&)> onVariablesToValueChanged{};
  /// Be notified when facts changed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsChanged{};
  /// Be notified about the punctual facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onPunctualFacts{};
  /// Be notified about the added facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsAdded{};
  /// Be notified about the removed facts.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsRemoved{};
  /// Be notified when goals changed.
  cpstd::observable::ObservableUnsafe<void (const std::map<int, std::vector<Goal>>&)> onGoalsChanged{};

  /**
  * @brief Add variables to value.
  * @param pVariablesToValue Variables to value to add.
  * @param pNow Current time.
  */
  void addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Add a fact.
  * @param pFact Fact to add.
  * @param pNow Current time.
  */
  bool addFact(const Fact& pFact,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Add several facts.
  * @param pSetOfFacts Facts to add.
  * @param pNow Current time.
  */
  template<typename FACTS>
  bool addFacts(const FACTS& pFacts,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Check if the world has a fact.
  bool hasFact(const Fact& pFact) const;

  /**
  * @brief Remove a fact.
  * @param pFact Fact to remove.
  * @param pNow Current time.
  */
  bool removeFact(const Fact& pFact,
                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Remove several facts.
  * @param pFacts Facts to remove.
  * @param pNow Current time.
  */
  template<typename FACTS>
  bool removeFacts(const FACTS& pFacts,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
  * @brief Modify some facts in the world.
  * @param pSetOfFacts Facts to modify in the world.
  * @param pNow Current time.
  */
  bool modifyFacts(const SetOfFacts& pSetOfFacts,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Set the facts of the world.
   * @param pFacts New set of facts for the world.
   * @param pNow Current time.
   */
  void setFacts(const std::set<Fact>& pFacts,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Fill accessible facts internally, according to a domain.
   * @param pDomain Domain containing the actions.
   */
  void fillAccessibleFacts(const Domain& pDomain);

  /**
   * @brief Can some facts modification become true, according to the world and the accessible facts stored internally in this object.
   * @param pSetOfFacts Set of facts to check if they are valid.
   * @return True if the facts are valid for the world, false otherwise.
   */
  bool canSetOfFactsBecomeTrue(const SetOfFacts& pSetOfFacts) const;

  /**
   * @brief Can some facts become true, according to the world and the accessible facts stored internally in this object.
   * @param pFacts Facts to check if they can become true.
   * @return True if the facts can become true, false otherwise.
   */
  bool canFactsBecomeTrue(const std::set<Fact>& pFacts) const;

  /**
   * @brief Are the facts contained in the problem.
   * @param[in] pSetOfFacts Facts to check.
   * @param[in] pPunctualFacts Punctual facts currently true.
   * @param[oui] pParametersPtr Parameters that the facts have in the problem.
   * @return True if all the fact are contained in the problem, false otherwise.
   */
  bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                    const std::set<Fact>& pPunctualFacts = {},
                    std::map<std::string, std::string>* pParametersPtr = nullptr) const;

  /// Facts of the world.
  const std::set<Fact>& facts() const { return _facts; }
  /// Facts name to number of occurences in the world.
  const std::map<std::string, std::size_t>& factNamesToNbOfOccurences() const { return _factNamesToNbOfOccurences; }
  /// Map of variables to value.
  const std::map<std::string, std::string>& variablesToValue() const { return _variablesToValue; }




  // Goal stack
  // ----------

  /// Get the goal serialized in string in top of the stack of goals, an empty string is returned if there is no goal.
  std::string getCurrentGoalStr() const;

  /// Get the goal in top of the stack of goals, nullptr is returned if there is no goal.
  const Goal* getCurrentGoalPtr() const;

  /**
   * @brief Is an optional fact satisfied in the problem.
   * @param pFactOptional Input optional fact.
   * @return True if the input optional fact is satisfied, false otherwise.
   */
  bool isOptionalFactSatisfied(const FactOptional& pFactOptional) const;

  /**
   * @brief Iterate on goals and remove non persistent goals.
   * @param pManageGoal Callback to manage the goal. If the callback returns true we stop the iteration.
   * @param pNow Current time.
   */
  void iterateOnGoalsAndRemoveNonPersistent(const std::function<bool (Goal&, int)>& pManageGoal,
                                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Default priority.
  static const int defaultPriority;

  /**
   * @brief Set the goals.
   * @param pGoals Map of priority to the associated goals.
   * @param pNow Current time.
   */
  void setGoals(const std::map<int, std::vector<Goal>>& pGoals,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Set the goals.
   * @param pGoals Set of goals.
   * @param pNow Current time.
   * @param pPriority Priority of the goals.
   */
  void setGoals(const std::vector<Goal>& pGoals,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                int pPriority = defaultPriority);

  /**
   * @brief Add some goals.
   * @param pGoals Map of priority to the associated goals.
   * @param pNow Current time.
   */
  void addGoals(const std::map<int, std::vector<Goal>>& pGoals,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Add some goals.
   * @param pGoals Set of goals to add.
   * @param pNow Current time.
   * @param pPriority Priority of the goals to add.
   */
  void addGoals(const std::vector<Goal>& pGoals,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                int pPriority = defaultPriority);

  /**
   * @brief Add a goal in front of the existing goals that have the same level of priority.
   * @param pGoal Goal to add.
   * @param pNow Current time.
   * @param pPriorityPriority of the goal to add.
   */
  void pushFrontGoal(const Goal& pGoal,
                     const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                     int pPriority = defaultPriority);

  /**
   * @brief Add a goal on bottom of the existing goals that have the same level of priority.
   * @param pGoal Goal to add.
   * @param pNow Current time.
   * @param pPriorityPriority of the goal to add.
   */
  void pushBackGoal(const Goal& pGoal,
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
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove some goals.
   * @param pGoalGroupId Group identifier of the goals to remove.
   * @param pNow Current time.
   */
  void removeGoals(const std::string& pGoalGroupId,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove the first goals that are already satisfied.
   * @param pNow Current time.
   */
  void removeFirstGoalsThatAreAlreadySatisfied(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Check if a goal is satisfied.<br/>
   * A goal is not satisfied if the condition is true (if it exist) and if the value of the goal is not true.
   * @param pGoal Goal to check.
   * @return True if the goal is satisfied.
   */
  bool isGoalSatisfied(const Goal& pGoal) const;

  /// Goals to satisfy.
  const std::map<int, std::vector<Goal>>& goals() const { return _goals; }

  /**
   * @brief Get the not satisfied goals.<br/>
   * A goal is not satisfied if the condition is true (if it exist) and if the value of the goal is not true.
   * @return Map of priority to not satisfied goals.
   */
  std::map<int, std::vector<Goal>> getNotSatisfiedGoals() const;


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
                          const std::shared_ptr<const SetOfInferences>& pSetOfInferences) { _setOfInferences.emplace(pSetOfInferencesId, pSetOfInferences); }

  /**
   * @brief Remove a set of inferences.
   * @param pSetOfInferencesId Identifier of the set of inferences to remove.
   *
   * If the inference is not found, this function will have no effect.
   * No exception will be raised.
   */
  void removeSetOfInferences(const SetOfInferencesId& pSetOfInferencesId);

  /// Get the set of inferences.
  const std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>>& getSetOfInferences() const { return _setOfInferences; }


  // Historical of actions done
  // --------------------------

  /// Historical of actions done.
  Historical historical{};


private:
  /// Map of priority to goals.
  std::map<int, std::vector<Goal>> _goals{};
  /// Variables to value.
  std::map<std::string, std::string> _variablesToValue{};
  /// Facts of the world.
  std::set<Fact> _facts{};
  /// Fact names to number of occurences in the world.
  std::map<std::string, std::size_t> _factNamesToNbOfOccurences{};
  /// Facts that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFacts{};
  /// Facts with values that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFactsWithAnyValues{};
  /// Facts that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFacts{};
  /// Know if we need to add accessible facts.
  bool _needToAddAccessibleFacts = true;
  /// Map set of inferences identifiers to the set of inferences.
  std::map<SetOfInferencesId, std::shared_ptr<const SetOfInferences>> _setOfInferences{};
  /// Current active goal.
  const Goal* _currentGoalPtr = nullptr;

  /// Stored what changed.
  struct WhatChanged
  {
    /// Punctual facts that are pinged.
    std::set<Fact> punctualFacts;
    /// Facts that we added in the world.
    std::set<Fact> addedFacts;
    /// Facts that we removed in the world.
    std::set<Fact> removedFacts;
    /// True if variable to values changed.
    bool variablesToValue = false;
    /// True if the goals changed.
    bool goals = false;

    /// Check if something changed.
    bool somethingChanged() const { return !punctualFacts.empty() || !addedFacts.empty() || !removedFacts.empty() || variablesToValue || goals; }
    /// Has some facts to add or to remove.
    bool hasFactsModifications() const { return !addedFacts.empty() || !removedFacts.empty(); }
  };

  /**
   * @brief Iterate on goals and remove non persistent goals.
   * @param[out] pWhatChanged Get what changed.
   * @param pManageGoal Callback to manage the goal. If the callback returns true we stop the iteration.
   * @param[in] pNow Current time.
   */
  void _iterateOnGoalsAndRemoveNonPersistent(WhatChanged& pWhatChanged,
                                             const std::function<bool (Goal&, int)>& pManageGoal,
                                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

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
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Clear accessible and removable facts.
  void _clearAccessibleAndRemovableFacts();

  /**
   * @brief Add an occurence to a fact name.
   * @param pFactName Fact name to add an occurence.
   */
  void _addFactNameRef(const std::string& pFactName);

  /**
   * @brief Remove no stackable goals.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _removeNoStackableGoals(WhatChanged& pWhatChanged,
                               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Feed accessible facts from a set of actions.
   * @param pActions Set of actions.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                           const Domain& pDomain);

  /**
   * @brief Feed accessible facts from a set of inferences.
   * @param pInferences Set of inferences.
   * @param pAllInferences Set of all the possible inferences.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                               const std::map<InferenceId, Inference>& pAllInferences,
                                               const Domain& pDomain);

  /**
   * @brief Feed accessible facts from a condition and an effect.
   * @param pCondition condition to check.
   * @param pEffect Effect to aply.
   * @param pParameters Parameter names.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromDeduction(const SetOfFacts& pCondition,
                                        const WorldModification& pEffect,
                                        const std::vector<std::string>& pParameters,
                                        const Domain& pDomain);

  /**
   * @brief Feed accessible facts from a fact.
   * @param pFact A fact.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromFact(const Fact& pFact,
                                   const Domain& pDomain);

  /**
   * @brief Feed accessible facts from a negated fact.
   * @param pFact A negated fact.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedAccessibleFactsFromNotFact(const Fact& pFact,
                                      const Domain& pDomain);

  /**
   * @brief Modify some facts in the world.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pSetOfFacts Facts to modify in the world.
   * @param[in] pNow Current time.
   */
  void _modifyFacts(WhatChanged& pWhatChanged,
                    const SetOfFacts& pSetOfFacts,
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Add some goals.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pGoals Set of goals to add.
   * @param[in] pNow Current time.
   */
  void _addGoals(WhatChanged& pWhatChanged,
                 const std::map<int, std::vector<Goal>>& pGoals,
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
                             const std::set<InferenceId>& pInferenceIds,
                             const std::map<InferenceId, Inference>& pInferences,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Do inferences and raise the observables if some facts or goals changed.
   * @param[in] pWhatChanged Get what changed.
   * @param[in] pNow Current time.
   */
  void _notifyWhatChanged(WhatChanged& pWhatChanged,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
