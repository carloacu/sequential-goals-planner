#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/factmodification.hpp>
#include <contextualplanner/types/goal.hpp>


namespace cp
{


// Specification of a modification of the world.
struct CONTEXTUALPLANNER_API WorldModification
{
  /// Construct a world modification without arguments.
  WorldModification()
    : factsModifications(),
      potentialFactsModifications(),
      goalsToAdd(),
      goalsToAddInCurrentPriority()
  {
  }

  /**
   * @brief Construct a world modification from the set of facts.
   * @param pSetOfFacts Set of facts to add in the world modification.
   */
  WorldModification(std::unique_ptr<cp::FactModification> pFactsModifications,
                    std::unique_ptr<cp::FactModification> pPotentialFactsModifications = std::unique_ptr<cp::FactModification>())
    : factsModifications(pFactsModifications ? std::move(pFactsModifications): std::unique_ptr<cp::FactModification>()),
      potentialFactsModifications(pPotentialFactsModifications ? std::move(pPotentialFactsModifications) : std::unique_ptr<cp::FactModification>()),
      goalsToAdd(),
      goalsToAddInCurrentPriority()
  {
  }

  WorldModification(const WorldModification& pOther)
    : factsModifications(pOther.factsModifications ? pOther.factsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>()),
      potentialFactsModifications(pOther.potentialFactsModifications ? pOther.potentialFactsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>()),
      goalsToAdd(pOther.goalsToAdd),
      goalsToAddInCurrentPriority(pOther.goalsToAddInCurrentPriority)
  {
  }

  void operator=(const WorldModification& pOther)
  {
    factsModifications = pOther.factsModifications ? pOther.factsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>();
    potentialFactsModifications = pOther.potentialFactsModifications ? pOther.potentialFactsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>();
    goalsToAdd = pOther.goalsToAdd;
    goalsToAddInCurrentPriority = pOther.goalsToAddInCurrentPriority;
  }

  /// Is the world modification empty.
  bool empty() const { return !factsModifications && !potentialFactsModifications && goalsToAdd.empty() && goalsToAddInCurrentPriority.empty(); }

  /// Check equality with another world modification.
  bool operator==(const WorldModification& pOther) const
  { return factsModifications == pOther.factsModifications && potentialFactsModifications == pOther.potentialFactsModifications &&
        goalsToAdd == pOther.goalsToAdd && goalsToAddInCurrentPriority == pOther.goalsToAddInCurrentPriority; }

  /// Check if this object contains a fact.
  bool hasFact(const cp::Fact& pFact) const;

  /**
   * @brief Add the content of another world modifiation.
   * @param pOther The other world modification to add.
   */
  void add(const WorldModification& pOther);

  /**
   * @brief Replace a fact by another inside this object.
   * @param pOldFact Fact to replace.
   * @param pNewFact New fact value to set.
   */
  void replaceFact(const cp::Fact& pOldFact,
                   const cp::Fact& pNewFact);

  /**
   * @brief Iterate over all the facts.
   * @param pCallback Callback called for each fact.
   */
  void forAllFacts(const std::function<void(const cp::Fact&)>& pCallback,
                   const Problem& pProblem) const;

  /**
   * @brief Iterate over all the facts until one callback returns true.
   * @param pCallback Callback called for each fact until one returns true.
   * @return True if one callback returned true, false otherwise.
   */
  bool forAllFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback,
                            const Problem& pProblem) const;

  /**
   * @brief Iterate over all the facts that should be removed.
   * @param pCallback Callback called for each fact that should be removed.
   */
  void forAllNotFacts(const std::function<void(const cp::Fact&)>& pCallback,
                      const Problem& pProblem) const;

  /**
   * @brief Iterate over all the facts that should be removed until one callback returns true.
   * @param pCallback Callback called for each fact that should be removed until one returns true.
   * @return True if one callback returned true, false otherwise.
   */
  bool forAllNotFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback,
                               const Problem& pProblem) const;

  /// Fact modifications declared and that will be applied to the world.
  std::unique_ptr<cp::FactModification> factsModifications;
  /// Fact modifications declared but that will not be applied to the world.
  std::unique_ptr<cp::FactModification> potentialFactsModifications;
  /// Goal priorities to goals to add in the world.
  std::map<int, std::vector<cp::Goal>> goalsToAdd;
  /// Goals to add in current priority in the world.
  std::vector<cp::Goal> goalsToAddInCurrentPriority;
};




// Implemenation

inline bool WorldModification::hasFact(const cp::Fact& pFact) const
{
  if ((factsModifications && factsModifications->hasFact(pFact)) ||
      (potentialFactsModifications && potentialFactsModifications->hasFact(pFact)))
    return true;
  for (const auto& currGoalWithPriority : goalsToAdd)
    for (const auto& currGoal : currGoalWithPriority.second)
      if (currGoal.factCondition().hasFact(pFact))
        return true;
  for (const auto& currGoal : goalsToAddInCurrentPriority)
    if (currGoal.factCondition().hasFact(pFact))
      return true;
  return false;
}

inline void WorldModification::add(const WorldModification& pOther)
{
  if (pOther.factsModifications)
  {
    if (factsModifications)
      factsModifications = FactModification::merge(*factsModifications, *pOther.factsModifications);
    else
      factsModifications = pOther.factsModifications->clone(nullptr);
  }
  if (pOther.potentialFactsModifications)
  {
    if (potentialFactsModifications)
      potentialFactsModifications = FactModification::merge(*potentialFactsModifications, *pOther.potentialFactsModifications);
    else
      potentialFactsModifications = pOther.potentialFactsModifications->clone(nullptr);
  }
  goalsToAdd.insert(pOther.goalsToAdd.begin(), pOther.goalsToAdd.end());
  goalsToAddInCurrentPriority.insert(goalsToAddInCurrentPriority.end(), pOther.goalsToAddInCurrentPriority.begin(), pOther.goalsToAddInCurrentPriority.end());
}

inline void WorldModification::replaceFact(const cp::Fact& pOldFact,
                                           const cp::Fact& pNewFact)
{
  if (factsModifications)
    factsModifications->replaceFact(pOldFact, pNewFact);
  if (potentialFactsModifications)
    potentialFactsModifications->replaceFact(pOldFact, pNewFact);
  for (auto& currGoalWithPriority : goalsToAdd)
    for (auto& currGoal : currGoalWithPriority.second)
      currGoal.factCondition().replaceFact(pOldFact, pNewFact);
  for (auto& currGoal : goalsToAddInCurrentPriority)
    currGoal.factCondition().replaceFact(pOldFact, pNewFact);
}

inline void WorldModification::forAllFacts(const std::function<void(const cp::Fact&)>& pCallback,
                                           const Problem& pProblem) const
{
  if (factsModifications)
    factsModifications->forAllFacts(pCallback, pProblem);
  if (potentialFactsModifications)
    potentialFactsModifications->forAllFacts(pCallback, pProblem);
}

inline bool WorldModification::forAllFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback,
                                                    const Problem& pProblem) const
{
  if (factsModifications && factsModifications->forAllFactsUntilTrue(pCallback, pProblem))
    return true;
  if (potentialFactsModifications && potentialFactsModifications->forAllFactsUntilTrue(pCallback, pProblem))
    return true;
  return false;
}

inline void WorldModification::forAllNotFacts(const std::function<void(const cp::Fact&)>& pCallback,
                                              const Problem& pProblem) const
{
  if (factsModifications)
    factsModifications->forAllNotFacts(pCallback, pProblem);
  if (potentialFactsModifications)
    potentialFactsModifications->forAllNotFacts(pCallback, pProblem);
}

inline bool WorldModification::forAllNotFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback,
                                                       const Problem& pProblem) const
{
  if (factsModifications && factsModifications->forAllNotFactsUntilTrue(pCallback, pProblem))
    return true;
  if (potentialFactsModifications && potentialFactsModifications->forAllNotFactsUntilTrue(pCallback, pProblem))
    return true;
  return false;
}


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP
