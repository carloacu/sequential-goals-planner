#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATECACHE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATECACHE_HPP

#include <map>
#include <memory>
#include <set>
#include <contextualplanner/types/fact.hpp>
#include <contextualplanner/util/alias.hpp>


namespace cp
{
struct Domain;
struct Inference;
struct FactsAlreadyChecked;
struct WorldState;
struct WorldStateModification;


struct CONTEXTUALPLANNER_API WorldStateCache
{
  /// Construct a world state.
  WorldStateCache(const WorldState& pWorldState);
  /// Construct a world state from another world state.
  WorldStateCache(const WorldState& pWorldState,
                  const WorldStateCache& pOther);

  void notifyAboutANewFact(const Fact& pNewFact);

  void refreshIfNeeded(const Domain& pDomain,
                       const std::set<Fact>& pFacts);

  /// Clear accessible and removable facts.
  void clear();

  const std::set<Fact>& accessibleFacts() const { return _accessibleFacts; }
  const std::set<Fact>& accessibleFactsWithAnyValues() const { return _accessibleFactsWithAnyValues; }
  const std::set<Fact>& removableFacts() const { return _removableFacts; }
  const std::set<Fact>& removableFactsWithAnyValues() const { return _removableFactsWithAnyValues; }


private:
  const WorldState& _worldState;
  /// Facts that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFacts;
  /// Facts with any values that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFactsWithAnyValues;
  /// Facts that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFacts;
  /// Facts with any values that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFactsWithAnyValues;
  /// Know if we need to add accessible facts.
  std::string _uuidOfLastDomainUsed;


  /**
   * @brief Feed accessible facts from a set of actions.
   * @param[in] pActions Set of actions.
   * @param[in] pDomain Domain containing all the possible actions and inferences.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromSetOfActions(const std::set<ActionId>& pActions,
                                            const Domain& pDomain,
                                            FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a set of inferences.
   * @param[in] pInferences Set of inferences.
   * @param[in] pAllInferences All inferences to consider.
   * @param[in] pDomain Domain containing all the possible actions and inferences.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromSetOfInferences(const std::set<InferenceId>& pInferences,
                                               const std::map<InferenceId, Inference>& pAllInferences,
                                               const Domain& pDomain,
                                               FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a condition and an effect.
   * @param[in] pEffect Effect to apply.
   * @param[in] pParameters Parameters of the condition and effect.
   * @param[in] pDomain Domain containing all the possible actions and inferences.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromDeduction(const WorldStateModification& pEffect,
                                         const std::vector<Parameter>& pParameters,
                                         const Domain& pDomain,
                                         FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a fact.
   * @param[in] pFact A fact.
   * @param[in] pDomain Domain containing all the possible actions and inferences.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromFact(const Fact& pFact,
                                    const Domain& pDomain,
                                    FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a negated fact.
   * @param[in] pFact A negated fact.
   * @param[in] pDomain Domain containing all the possible actions and inferences.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromNotFact(const Fact& pFact,
                                       const Domain& pDomain,
                                       FactsAlreadyChecked& pFactsAlreadychecked);
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_WORLDSTATECACHE_HPP
