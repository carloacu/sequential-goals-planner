#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP

#include <set>
#include "../util/api.hpp"
#include <contextualplanner/types/historical.hpp>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/setoffacts.hpp>
#include <contextualplanner/util/observableunsafe.hpp>


namespace cp
{
struct Domain;


/// Current world, goal for the world and historical of actions done.
struct CONTEXTUALPLANNER_API Problem
{
  /// Construct a problem.
  Problem() = default;
  /// Construct a problem from another problem.
  Problem(const Problem& pOther);

  /**
   * @brief Notify that an action has been done.
   * @param pActionId Action identifier.
   * @param pParameters Parameters of the action done.
   * @param pEffect Effect of the action done.
   * @param pNow Current time.
   * @param pGoalsToAdd Goals to add.
   */
  void notifyActionDone(const std::string& pActionId,
                        const std::map<std::string, std::string>& pParameters,
                        const SetOfFacts& pEffect,
                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                        const std::map<int, std::vector<Goal>>* pGoalsToAdd);


  // World state
  // -----------

  /// Be notified when a variable to value changed.
  cpstd::observable::ObservableUnsafe<void (const std::map<std::string, std::string>&)> onVariablesToValueChanged{};
  /// Be notified when facts changed.
  cpstd::observable::ObservableUnsafe<void (const std::set<Fact>&)> onFactsChanged{};
  /// Be notified when goals changed.
  cpstd::observable::ObservableUnsafe<void (const std::map<int, std::vector<Goal>>&)> onGoalsChanged{};

  /// Add variables to value.
  void addVariablesToValue(const std::map<std::string, std::string>& pVariablesToValue);

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
   * @brief Fill reachable facts internally, according to a domain.
   * @param pDomain Domain containing the actions.
   */
  void fillReachableFacts(const Domain& pDomain);

  /**
   * @brief Can some facts become true, according to the world and the reachable facts stored internally in this object.
   * @param pSetOfFacts Set of facts to check if they can become true.
   * @return True if the facts can become true, false otherwise.
   */
  bool canFactsBecomeTrue(const SetOfFacts& pSetOfFacts) const;

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
   */
  void changeGoalPriority(const std::string& pGoalStr,
                          int pPriority,
                          bool pPushFrontOrBottomInCaseOfConflictWithAnotherGoal);

  /**
   * @brief Remove some goals.
   * @param pGoalGroupId Group identifier of the goals to remove.
   * @param pNow Current time.
   */
  void removeGoals(const std::string& pGoalGroupId,
                   const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Remove the first goals that are already satisfied.
  void removeFirstGoalsThatAreAlreadySatisfied();

  /// Goals to satisfy.
  const std::map<int, std::vector<Goal>>& goals() const { return _goals; }



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
  std::set<Fact> _reachableFacts{};
  /// Facts with values that can be reached with the set of actions of the domain.
  std::set<Fact> _reachableFactsWithAnyValues{};
  /// Facts that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFacts{};
  /// Know if we need to add reachable facts.
  bool _needToAddReachableFacts = true;

  /**
   * @brief Add facts without raising a notification.
   * @param pFacts Facts to add.
   * @param pNow Current time.
   * @return True if some facts were added, false otherwise.
   */
  template<typename FACTS>
  bool _addFactsWithoutFactNotification(const FACTS& pFacts,
                                        const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Remove facts without raising a notification.
   * @param pFacts Facts to add.
   * @param pNow Current time.
   * @return True if some facts were removed, false otherwise.
   */
  template<typename FACTS>
  bool _removeFactsWithoutFactNotification(const FACTS& pFacts,
                                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /// Clear reachable and removable facts.
  void _clearReachableAndRemovableFacts();

  /**
   * @brief Add an occurence to a fact name.
   * @param pFactName Fact name to add an occurence.
   */
  void _addFactNameRef(const std::string& pFactName);

  /**
   * @brief Remove no stackable goals.
   * @param pNow Current time.
   */
  void _removeNoStackableGoals(const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

  /**
   * @brief Feed reachable facts from a set of actions.
   * @param pActions Set of actions.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedReachableFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                           const Domain& pDomain);

  /**
   * @brief Feed reachable facts from a fact.
   * @param pFact A fact.
   * @param pDomain Domain containing all the possible actions.
   */
  void _feedReachableFacts(const Fact& pFact,
                           const Domain& pDomain);
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEM_HPP
