#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP

#include <functional>
#include <map>
#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/setoffacts.hpp>


namespace cp
{


// Specification of a modification of the world.
struct CONTEXTUALPLANNER_API WorldModification
{
  /// Construct a world modification without arguments.
  WorldModification()
    : factsModifications(),
      potentialFactsModifications(),
      goalsToAdd()
  {
  }

  /**
   * @brief Construct a world modification.
   * @param[in] pFacts Facts to add in the world.
   * @param[in] pNotFacts Facts to remove in the world.
   */
  WorldModification(const std::initializer_list<Fact>& pFacts,
                    const std::initializer_list<Fact>& pNotFacts = {})
    : factsModifications(pFacts, pNotFacts),
      potentialFactsModifications(),
      goalsToAdd()
  {
  }

  /**
   * @brief Construct a world modification from the set of facts.
   * @param pSetOfFacts Set of facts to add in the world modification.
   */
  WorldModification(const SetOfFacts& pSetOfFacts)
    : factsModifications(pSetOfFacts),
      potentialFactsModifications(),
      goalsToAdd()
  {
  }

  /// Is the world modification empty.
  bool empty() const { return factsModifications.empty() && potentialFactsModifications.empty() && goalsToAdd.empty(); }

  /// Check equality with another world modification.
  bool operator==(const WorldModification& pOther) const
  { return factsModifications == pOther.factsModifications && potentialFactsModifications == pOther.potentialFactsModifications && goalsToAdd == pOther.goalsToAdd; }

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
  void forAllFacts(const std::function<void(const cp::Fact&)>& pCallback) const;

  /**
   * @brief Iterate over all the facts until one callback returns true.
   * @param pCallback Callback called for each fact until one returns true.
   * @return True if one callback returned true, false otherwise.
   */
  bool forAllFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback) const;

  /**
   * @brief Iterate over all the facts that should be removed.
   * @param pCallback Callback called for each fact that should be removed.
   */
  void forAllNotFacts(const std::function<void(const cp::Fact&)>& pCallback) const;

  /**
   * @brief Iterate over all the facts that should be removed until one callback returns true.
   * @param pCallback Callback called for each fact that should be removed until one returns true.
   * @return True if one callback returned true, false otherwise.
   */
  bool forAllNotFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback) const;

  /// Fact modifications declared and that will be applied to the world.
  cp::SetOfFacts factsModifications;
  /// Fact modifications declared but that will not be applied to the world.
  cp::SetOfFacts potentialFactsModifications;
  /// Goals to add in the world.
  std::map<int, std::vector<cp::Goal>> goalsToAdd;
};




// Implemenation

inline bool WorldModification::hasFact(const cp::Fact& pFact) const
{
  if (factsModifications.hasFact(pFact) ||
      potentialFactsModifications.hasFact(pFact))
    return true;
  for (const auto& currGoalWithPriority : goalsToAdd)
    for (const auto& currGoal : currGoalWithPriority.second)
      if (currGoal.factOptional().fact == pFact)
        return true;
  return false;
}

inline void WorldModification::add(const WorldModification& pOther)
{
  factsModifications.add(pOther.factsModifications);
  potentialFactsModifications.add(pOther.potentialFactsModifications);
  goalsToAdd.insert(pOther.goalsToAdd.begin(), pOther.goalsToAdd.end());
}

inline void WorldModification::replaceFact(const cp::Fact& pOldFact,
                                           const cp::Fact& pNewFact)
{
  factsModifications.replaceFact(pOldFact, pNewFact);
  potentialFactsModifications.replaceFact(pOldFact, pNewFact);
  for (auto& currGoalWithPriority : goalsToAdd)
    for (auto& currGoal : currGoalWithPriority.second)
      if (currGoal.factOptional().fact == pOldFact)
        currGoal.factOptional().fact = pNewFact;
}

inline void WorldModification::forAllFacts(const std::function<void(const cp::Fact&)>& pCallback) const
{
  for (auto& currFact : factsModifications.facts)
    pCallback(currFact);
  for (auto& currFact : potentialFactsModifications.facts)
    pCallback(currFact);
}

inline bool WorldModification::forAllFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback) const
{
  for (auto& currFact : factsModifications.facts)
    if (pCallback(currFact))
      return true;
  for (auto& currFact : potentialFactsModifications.facts)
    if (pCallback(currFact))
      return true;
  return false;
}

inline void WorldModification::forAllNotFacts(const std::function<void(const cp::Fact&)>& pCallback) const
{
  for (auto& currFact : factsModifications.notFacts)
    pCallback(currFact);
  for (auto& currFact : potentialFactsModifications.notFacts)
    pCallback(currFact);
}

inline bool WorldModification::forAllNotFactsUntilTrue(const std::function<bool(const cp::Fact&)>& pCallback) const
{
  for (auto& currFact : factsModifications.notFacts)
    if (pCallback(currFact))
      return true;
  for (auto& currFact : potentialFactsModifications.notFacts)
    if (pCallback(currFact))
      return true;
  return false;
}


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDMODIFICATION_HPP
