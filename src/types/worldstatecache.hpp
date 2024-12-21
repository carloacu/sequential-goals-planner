#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_WORLDSTATECACHE_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_WORLDSTATECACHE_HPP

#include <map>
#include <memory>
#include <set>
#include <orderedgoalsplanner/types/fact.hpp>
#include <orderedgoalsplanner/types/factstovalue.hpp>
#include <orderedgoalsplanner/types/setoffacts.hpp>
#include <orderedgoalsplanner/util/alias.hpp>


namespace ogp
{
struct Domain;
struct Event;
struct FactsAlreadyChecked;
struct WorldState;
struct WorldStateModification;


struct ORDEREDGOALSPLANNER_API WorldStateCache
{
  /// Construct a world state.
  WorldStateCache(const WorldState& pWorldState);
  /// Construct a world state from another world state.
  WorldStateCache(const WorldState& pWorldState,
                  const WorldStateCache& pOther);

  void notifyAboutANewFact(const Fact& pNewFact);

  void refreshIfNeeded(const Domain& pDomain,
                       const std::map<Fact, bool>& pFacts);

  /// Clear accessible and removable facts.
  void clear();

  const SetOfFacts& accessibleFacts() const { return _accessibleFacts; } // TODO: remove?
  const std::set<Fact>& accessibleFactsWithAnyValues() const { return _accessibleFactsWithAnyValues; } // TODO: remove?
  const SetOfFacts& removableFacts() const { return _removableFacts; } // TODO: remove?
  const std::set<Fact>& removableFactsWithAnyValues() const { return _removableFactsWithAnyValues; } // TODO: remove?


private:
  const WorldState& _worldState;
  /// Facts that can be reached with the set of actions of the domain.
  SetOfFacts _accessibleFacts;
  /// Facts with any values that can be reached with the set of actions of the domain.
  std::set<Fact> _accessibleFactsWithAnyValues;
  /// Facts that can be removed with the set of actions of the domain.
  SetOfFacts _removableFacts;
  /// Facts with any values that can be removed with the set of actions of the domain.
  std::set<Fact> _removableFactsWithAnyValues;
  /// Know if we need to add accessible facts.
  std::string _uuidOfLastDomainUsed;


  /**
   * @brief Feed accessible facts from a set of actions.
   * @param[in] pActions Set of actions.
   * @param[in] pDomain Domain containing all the possible actions and events.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromSetOfActions(const FactsToValue::ConstMapOfFactIterator& pActions,
                                            const Domain& pDomain,
                                            FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a set of events.
   * @param[in] pEvents Set of events.
   * @param[in] pAllEvents All events to consider.
   * @param[in] pDomain Domain containing all the possible actions and events.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromSetOfEvents(const FactsToValue::ConstMapOfFactIterator& pEvents,
                                           const std::map<EventId, Event>& pAllEvents,
                                           const Domain& pDomain,
                                           FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a condition and an effect.
   * @param[in] pEffect Effect to apply.
   * @param[in] pParameters Parameters of the condition and effect.
   * @param[in] pDomain Domain containing all the possible actions and events.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromDeduction(const WorldStateModification& pEffect,
                                         const std::vector<Parameter>& pParameters,
                                         const Domain& pDomain,
                                         FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a fact.
   * @param[in] pFact A fact.
   * @param[in] pDomain Domain containing all the possible actions and events.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromFact(const Fact& pFact,
                                    const Domain& pDomain,
                                    FactsAlreadyChecked& pFactsAlreadychecked);

  /**
   * @brief Feed accessible facts from a negated fact.
   * @param[in] pFact A negated fact.
   * @param[in] pDomain Domain containing all the possible actions and events.
   * @param[in, out] pFactsAlreadychecked Cache of fact already checked to not loop forever.
   */
  void _feedAccessibleFactsFromNotFact(const Fact& pFact,
                                       const Domain& pDomain,
                                       FactsAlreadyChecked& pFactsAlreadychecked);
};

} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_WORLDSTATECACHE_HPP
