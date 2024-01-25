#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP

#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/util/observableunsafe.hpp>
#include "../util/api.hpp"


namespace cp
{
struct Domain;
struct Goal;
struct GoalStack;
struct Inference;
struct OneStepOfPlannerResult;
struct SetOfInferences;
struct WorldStateModification;
struct WorldStateCache;

/**
 * @brief Current state of the world.<br/>
 * It is composed of a set of facts.<br/>
 * It also has accessors to optimize algorithms that will use this world state.
 */
struct CONTEXTUALPLANNER_API WorldState
{
  /// Construct a world state.
  WorldState();
  /// Construct a world state from another world state.
  WorldState(const WorldState& pOther);

  ~WorldState();

  void operator=(const WorldState& pOther);

  /**
   * @brief Notify that an action has been done.
   * @param[in] pOneStepOfPlannerResult Planner result step that motivated this action.
   * @param[in] pEffect Effect of the done action.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  void notifyActionDone(const OneStepOfPlannerResult& pOneStepOfPlannerResult,
                        const std::unique_ptr<WorldStateModification>& pEffect,
                        GoalStack& pGoalStack,
                        const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  /// Be notified when facts changed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsChanged;
  /// Be notified when punctual facts changed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onPunctualFacts;
  /// Be notified when facts are added.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsAdded;
  /// Be notified when facts are removed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsRemoved;

  /**
   * @brief Add a fact.
   * @param[in] pFact Fact to add.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  bool addFact(const Fact& pFact,
               GoalStack& pGoalStack,
               const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Add several facts.
   * @param[in] pFacts Facts to add.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  template<typename FACTS>
  bool addFacts(const FACTS& pFacts,
                GoalStack& pGoalStack,
                const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Check if the world has a fact or the negation of the fact.
  bool hasFact(const Fact& pFact) const;

  /**
   * @brief Remove a fact.
   * @param[in] pFact Fact to remove.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
  */
  bool removeFact(const Fact& pFact,
                  GoalStack& pGoalStack,
                  const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove several facts.
   * @param[in] pFacts Facts to remove.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  template<typename FACTS>
  bool removeFacts(const FACTS& pFacts,
                   GoalStack& pGoalStack,
                   const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Modify the world state.
   * @param[in] pWsModif Modification to do.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  bool modify(const std::unique_ptr<WorldStateModification>& pWsModif,
              GoalStack& pGoalStack,
              const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Set the facts of the world.
   * @param[in] pFacts New set of facts for the world.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  void setFacts(const std::set<Fact>& pFacts,
                GoalStack& pGoalStack,
                const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Can a fact become true, according to the world and the accessible facts stored internally in this object.
   * @param[in] pFact Fact to check if it can become true.
   * @return True if the fact can become true, false otherwise.
   */
  bool canFactBecomeTrue(const Fact& pFact) const;

  /**
   * @brief Can a fact be modified according to the cache of accessible facts.
   * @param[in] pFactName Name of the fact to check.
   * @return True if the fact can be modified according to the cache of accessible facts.
   */
  bool canFactNameBeModified(const std::string& pFactName) const;

  /**
   * @brief Get the value of a fact in the world state.
   * @param[in] pFact Fact to extract the value.
   * @return The value of the fact in the world state, an empty string if the fact is not in the world state.
   */
  std::string getFactValue(const Fact& pFact) const;

  /**
   * @brief Extract the potential arguments of a fact parameter.
   * @param[out] pPotentialArgumentsOfTheParameter The extracted the potential arguments of a fact parameter.
   * @param[in] pFact Fact to consider for the parameter.
   * @param[in] pParameter Parameter to consider in the fact.
   */
  void extractPotentialArgumentsOfAFactParameter(
      std::set<Fact>& pPotentialArgumentsOfTheParameter,
      const Fact& pFact,
      const std::string& pParameter) const;

  /// Facts of the world.
  const std::set<Fact>& facts() const { return _facts; }
  /// Fact names to facts in the world.
  const std::map<std::string, std::set<Fact>>& factNamesToFacts() const { return _factNamesToFacts; }


  /**
   * @brief Is an optional fact satisfied.
   * @param[in] pFactOptional Input optional fact.
   * @return True if the optional fact is satisfied, false otherwise.
   */
  bool isOptionalFactSatisfied(const FactOptional& pFactOptional) const;


  /**
   * @brief Is an optional fact satisfied in a specific context.
   * @param[in] pFactOptional Input optional fact.
   * @param[in] pPunctualFacts Punctual fact of the context.
   * @param[in] pRemovedFacts Facts currently removed in the context.
   * @param[in, out] pParametersToPossibleArgumentsPtr Map of parameters to possible values to extract.
   * @param[out] pCanBecomeTruePtr If this optional fact can become true according to the facts that can become true.
   * @return True if the optional fact is satisfied, false otherwise.
   */
  bool isOptionalFactSatisfiedInASpecificContext(const FactOptional& pFactOptional,
                                                 const std::set<Fact>& pPunctualFacts,
                                                 const std::set<Fact>& pRemovedFacts,
                                                 std::map<std::string, std::set<std::string>>* pParametersToPossibleArgumentsPtr,
                                                 bool* pCanBecomeTruePtr) const;

  /**
   * @brief Check if a goal is satisfied.<br/>
   * A goal is satisfied if his internal objective condition is satisfied and if the goal is enabled.
   * @param[in] pGoal Goal to check.
   * @return True if the goal is satisfied.
   */
  bool isGoalSatisfied(const Goal& pGoal) const;


  void refreshCacheIfNeeded(const Domain& pDomain);

  const std::set<Fact>& removableFacts() const;


private:
  /// Facts of the world state.
  std::set<Fact> _facts;
  /// Fact names to facts in the world state.
  std::map<std::string, std::set<Fact>> _factNamesToFacts;
  std::unique_ptr<WorldStateCache> _cache;

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
    bool hasFactsToModifyInTheWorldForSure() const { return !addedFacts.empty() || !removedFacts.empty(); }
  };


  /**
   * @brief Add facts without inference deduction and raising a notification.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pFacts Facts to add.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pNow Current time.
   * @return True if some facts were added, false otherwise.
   */
  template<typename FACTS>
  void _addFacts(WhatChanged& pWhatChanged,
                 const FACTS& pFacts,
                 GoalStack& pGoalStack,
                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove facts without inference deduction and raising a notification.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pFacts Facts to remove.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pNow Current time.
   * @return True if some facts were removed, false otherwise.
   */
  template<typename FACTS>
  void _removeFacts(WhatChanged& pWhatChanged,
                    const FACTS& pFacts,
                    GoalStack& pGoalStack,
                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Modify the world state without inference deduction and raising a notification.
   * @param[out] pWhatChanged Get what changed.
   * @param[in] pWsModif Modification to do.
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pNow Current time.
   */
  void _modify(WhatChanged& pWhatChanged,
               const std::unique_ptr<WorldStateModification>& pWsModif,
               GoalStack& pGoalStack,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Try to apply some inferences according to what changed in the world state.
   * @param[in, out] pInferencesAlreadyApplied Cache of inferences that we already considered.
   * @param[in, out] pWhatChanged What changed in the world state.
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
   * @param[out] pGoalStack Goal stacks that need to be refreshed.<br/>
   * For example the current goal of the stack can be satisfied now and so maybe it should be removed from the goal stack.
   * @param[in] pSetOfInferences Inferences to apply indirect modifications according to the inferences.
   * @param[in] pNow Current time.
   */
  void _notifyWhatChanged(WhatChanged& pWhatChanged,
                          GoalStack& pGoalStack,
                          const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


  friend struct ConditionNode;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATE_HPP
