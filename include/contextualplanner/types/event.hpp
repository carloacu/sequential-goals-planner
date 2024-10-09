#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_EVENT_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_EVENT_HPP

#include <assert.h>
#include <functional>
#include <map>
#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/condition.hpp>
#include <contextualplanner/types/worldstatemodification.hpp>
#include <contextualplanner/types/goal.hpp>

namespace cp
{

/// Specification what is an event.
struct CONTEXTUALPLANNER_API Event
{
  /// Construct an event.
  Event(std::unique_ptr<Condition> pCondition,
        std::unique_ptr<WorldStateModification> pFactsToModify,
        const std::vector<Parameter>& pParameters = {},
        const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd = {});

  /// Construct a copy.
  Event(const Event& pEvent)
    : parameters(pEvent.parameters),
      condition(pEvent.condition ? pEvent.condition->clone() : std::unique_ptr<Condition>()),
      factsToModify(pEvent.factsToModify ? pEvent.factsToModify->clone(nullptr) : std::unique_ptr<WorldStateModification>()),
      goalsToAdd(pEvent.goalsToAdd)
  {
    assert(condition);
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
   * The condition is true if the condition is a sub set of a corresponding world state.
   */
  const std::unique_ptr<Condition> condition;
  /// Facts to add or to remove if the condition is true.
  const std::unique_ptr<WorldStateModification> factsToModify;
  /// Goals to add if the condition is true.
  const std::map<int, std::vector<cp::Goal>> goalsToAdd;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_EVENT_HPP
