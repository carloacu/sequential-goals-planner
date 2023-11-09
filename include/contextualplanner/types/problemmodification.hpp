#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEMMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEMMODIFICATION_HPP

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
struct CONTEXTUALPLANNER_API ProblemModification
{
  /// Construct a world modification without arguments.
  ProblemModification()
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
  ProblemModification(std::unique_ptr<cp::FactModification> pFactsModifications,
                      std::unique_ptr<cp::FactModification> pPotentialFactsModifications = std::unique_ptr<cp::FactModification>())
    : factsModifications(pFactsModifications ? std::move(pFactsModifications): std::unique_ptr<cp::FactModification>()),
      potentialFactsModifications(pPotentialFactsModifications ? std::move(pPotentialFactsModifications) : std::unique_ptr<cp::FactModification>()),
      goalsToAdd(),
      goalsToAddInCurrentPriority()
  {
  }

  ProblemModification(const ProblemModification& pOther)
    : factsModifications(pOther.factsModifications ? pOther.factsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>()),
      potentialFactsModifications(pOther.potentialFactsModifications ? pOther.potentialFactsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>()),
      goalsToAdd(pOther.goalsToAdd),
      goalsToAddInCurrentPriority(pOther.goalsToAddInCurrentPriority)
  {
  }

  void operator=(const ProblemModification& pOther)
  {
    factsModifications = pOther.factsModifications ? pOther.factsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>();
    potentialFactsModifications = pOther.potentialFactsModifications ? pOther.potentialFactsModifications->clone(nullptr) : std::unique_ptr<cp::FactModification>();
    goalsToAdd = pOther.goalsToAdd;
    goalsToAddInCurrentPriority = pOther.goalsToAddInCurrentPriority;
  }

  /// Is the world modification empty.
  bool empty() const { return !factsModifications && !potentialFactsModifications && goalsToAdd.empty() && goalsToAddInCurrentPriority.empty(); }

  /// Check equality with another world modification.
  bool operator==(const ProblemModification& pOther) const;
  bool operator!=(const ProblemModification& pOther) const { return !operator==(pOther); }

  /// Check if this object contains a fact.
  bool hasFact(const cp::Fact& pFact) const;

  /**
   * @brief Add the content of another world modifiation.
   * @param pOther The other world modification to add.
   */
  void add(const ProblemModification& pOther);

  /**
   * @brief Replace a fact by another inside this object.
   * @param pOldFact Fact to replace.
   * @param pNewFact New fact value to set.
   */
  void replaceFact(const cp::Fact& pOldFact,
                   const cp::Fact& pNewFact);

  /**
   * @brief Iterate over all the optinal facts until one callback returns true.
   * @param pCallback Callback called for each optional fact until one returns true.
   * @return True if one callback returned true, false otherwise.
   */
  bool forAllUntilTrue(const std::function<bool(const cp::FactOptional&)>& pCallback,
                       const WorldState& pWorldState) const;
  /**
   * @brief Iterate over all the facts.
   * @param pCallback Callback called for each fact.
   */
  void forAll(const std::function<void(const cp::FactOptional&)>& pCallback,
              const WorldState& pWorldState) const;

  std::string factsModifications_str() const { return factsModifications ? factsModifications->toStr() : ""; }
  std::string potentialFactsModifications_str() const { return potentialFactsModifications ? potentialFactsModifications->toStr() : ""; }

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

inline bool ProblemModification::hasFact(const cp::Fact& pFact) const
{
  if ((factsModifications && factsModifications->hasFact(pFact)) ||
      (potentialFactsModifications && potentialFactsModifications->hasFact(pFact)))
    return true;
  for (const auto& currGoalWithPriority : goalsToAdd)
    for (const auto& currGoal : currGoalWithPriority.second)
      if (currGoal.objective().hasFact(pFact))
        return true;
  for (const auto& currGoal : goalsToAddInCurrentPriority)
    if (currGoal.objective().hasFact(pFact))
      return true;
  return false;
}

inline void ProblemModification::add(const ProblemModification& pOther)
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

inline void ProblemModification::replaceFact(const cp::Fact& pOldFact,
                                             const cp::Fact& pNewFact)
{
  if (factsModifications)
    factsModifications->replaceFact(pOldFact, pNewFact);
  if (potentialFactsModifications)
    potentialFactsModifications->replaceFact(pOldFact, pNewFact);
  for (auto& currGoalWithPriority : goalsToAdd)
    for (auto& currGoal : currGoalWithPriority.second)
      currGoal.objective().replaceFact(pOldFact, pNewFact);
  for (auto& currGoal : goalsToAddInCurrentPriority)
    currGoal.objective().replaceFact(pOldFact, pNewFact);
}

inline bool ProblemModification::forAllUntilTrue(const std::function<bool(const cp::FactOptional&)>& pCallback,
                                                 const WorldState& pWorldState) const
{
  if (factsModifications && factsModifications->forAllUntilTrue(pCallback, pWorldState))
    return true;
  if (potentialFactsModifications && potentialFactsModifications->forAllUntilTrue(pCallback, pWorldState))
    return true;
  return false;
}

inline void ProblemModification::forAll(const std::function<void(const cp::FactOptional&)>& pCallback,
                                        const WorldState& pWorldState) const
{
  if (factsModifications)
    factsModifications->forAll(pCallback, pWorldState);
  if (potentialFactsModifications)
    potentialFactsModifications->forAll(pCallback, pWorldState);
}



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_PROBLEMMODIFICATION_HPP
