#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_EVENT_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_EVENT_HPP

#include <assert.h>
#include <functional>
#include <map>
#include <vector>
#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/condition.hpp>
#include <prioritizedgoalsplanner/types/worldstatemodification.hpp>
#include <prioritizedgoalsplanner/types/goal.hpp>

namespace pgp
{

/// Specification what is an event.
struct PRIORITIZEDGOALSPLANNER_API Event
{
  /// Construct an event.
  Event(std::unique_ptr<Condition> pPrecondition,
        std::unique_ptr<WorldStateModification> pFactsToModify,
        const std::vector<Parameter>& pParameters = {},
        const std::map<int, std::vector<pgp::Goal>>& pGoalsToAdd = {});

  /// Construct a copy.
  Event(const Event& pEvent)
    : parameters(pEvent.parameters),
      precondition(pEvent.precondition ? pEvent.precondition->clone() : std::unique_ptr<Condition>()),
      factsToModify(pEvent.factsToModify ? pEvent.factsToModify->clone(nullptr) : std::unique_ptr<WorldStateModification>()),
      goalsToAdd(pEvent.goalsToAdd),
      actionsPredecessorsCache(pEvent.actionsPredecessorsCache),
      eventsPredecessorsCache(pEvent.eventsPredecessorsCache)
  {
    assert(precondition);
    assert(factsToModify || !goalsToAdd.empty());
  }

  void updateSuccessionCache(const Domain& pDomain,
                             const SetOfEventsId& pSetOfEventsIdOfThisEvent,
                             const EventId& pEventIdOfThisEvent);
  std::string printSuccessionCache() const;

  /// Parameter names of this event.
  std::vector<Parameter> parameters;
  /**
   * Condition to apply the facts and goals modification.
   * The precondition is true if the precondition is a sub set of a corresponding world state.
   */
  const std::unique_ptr<Condition> precondition;
  /// Facts to add or to remove if the condition is true.
  const std::unique_ptr<WorldStateModification> factsToModify;
  /// Goals to add if the condition is true.
  const std::map<int, std::vector<pgp::Goal>> goalsToAdd;

  std::set<ActionId> actionsPredecessorsCache;
  std::set<FullEventId> eventsPredecessorsCache;
};



} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_EVENT_HPP
