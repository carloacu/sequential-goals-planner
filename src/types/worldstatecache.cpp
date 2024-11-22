#include "worldstatecache.hpp"
#include <orderedgoalsplanner/types/domain.hpp>
#include <orderedgoalsplanner/types/worldstate.hpp>
#include "factsalreadychecked.hpp"


namespace ogp
{
namespace
{
std::string _noUuid = "noUuid";
}


WorldStateCache::WorldStateCache(const WorldState& pWorldState)
  : _worldState(pWorldState),
    _accessibleFacts(),
    _accessibleFactsWithAnyValues(),
    _removableFacts(),
    _removableFactsWithAnyValues(),
    _uuidOfLastDomainUsed(_noUuid)
{
}


WorldStateCache::WorldStateCache(const WorldState& pWorldState,
                                 const WorldStateCache& pOther)
  : _worldState(pWorldState),
    _accessibleFacts(pOther._accessibleFacts),
    _accessibleFactsWithAnyValues(pOther._accessibleFactsWithAnyValues),
    _removableFacts(pOther._removableFacts),
    _removableFactsWithAnyValues(pOther._removableFactsWithAnyValues),
    _uuidOfLastDomainUsed(pOther._uuidOfLastDomainUsed)
{
}


void WorldStateCache::clear()
{
  _accessibleFacts.clear();
  _accessibleFactsWithAnyValues.clear();
  _removableFacts.clear();
  _removableFactsWithAnyValues.clear();
  _uuidOfLastDomainUsed = _noUuid;
}


void WorldStateCache::notifyAboutANewFact(const Fact& pNewFact)
{
  auto itAccessible = _accessibleFacts.facts().find(pNewFact);
  if (itAccessible != _accessibleFacts.facts().end())
  {
    // If we already known that this fact was accessible no need to clear the cache.
    // We simply just remove it from the accessible fact,
    // because an accessible fact means that the fact is not already present in the world state.
    _accessibleFacts.erase(pNewFact);
  }
  else
  {
    clear();
  }
}


void WorldStateCache::refreshIfNeeded(const Domain& pDomain,
                                      const std::map<Fact, bool>& pFacts)
{
  if (_uuidOfLastDomainUsed == pDomain.getUuid())
    return;
  _uuidOfLastDomainUsed = pDomain.getUuid();

  for (int i = 0; i < 2; ++i) // 2 times to have all the accessible facts
  {
    FactsAlreadyChecked factsAlreadychecked;
    for (const auto& currFact : pFacts)
    {
      if (_accessibleFacts.facts().count(currFact.first) == 0)
      {
        auto itPrecToActions = pDomain.preconditionToActions().find(currFact.first);
        _feedAccessibleFactsFromSetOfActions(itPrecToActions, pDomain, factsAlreadychecked);
      }
    }
    auto actionWithoutPrecondition = pDomain.actionsWithoutFactToAddInPrecondition().valuesWithoutFact();
    _feedAccessibleFactsFromSetOfActions(actionWithoutPrecondition, pDomain,
                                         factsAlreadychecked);
  }
}


void WorldStateCache::_feedAccessibleFactsFromSetOfActions(const FactsToValue::ConstMapOfFactIterator& pActions,
                                                           const Domain& pDomain,
                                                           FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto& actions = pDomain.actions();
  for (const auto& currAction : pActions)
  {
    auto itAction = actions.find(currAction);
    if (itAction != actions.end())
    {
      const Action& action = itAction->second;
      if (!action.precondition || action.precondition->canBecomeTrue(_worldState, action.parameters))
      {
        if (action.effect.worldStateModification)
          _feedAccessibleFactsFromDeduction(*action.effect.worldStateModification, action.parameters,
                                            pDomain, pFactsAlreadychecked);
        if (action.effect.potentialWorldStateModification)
          _feedAccessibleFactsFromDeduction(*action.effect.potentialWorldStateModification, action.parameters,
                                            pDomain, pFactsAlreadychecked);
      }
    }
  }
}


void WorldStateCache::_feedAccessibleFactsFromSetOfEvents(const FactsToValue::ConstMapOfFactIterator& pEvents,
                                                          const std::map<EventId, Event>& pAllEvents,
                                                          const Domain& pDomain,
                                                          FactsAlreadyChecked& pFactsAlreadychecked)
{
  for (const auto& currEvent : pEvents)
  {
    auto itEvent = pAllEvents.find(currEvent);
    if (itEvent != pAllEvents.end())
    {
      const Event& event = itEvent->second;
      if (!event.precondition || event.precondition->canBecomeTrue(_worldState, event.parameters))
        _feedAccessibleFactsFromDeduction(*event.factsToModify, event.parameters,
                                          pDomain, pFactsAlreadychecked);
    }
  }
}


void WorldStateCache::_feedAccessibleFactsFromDeduction(const WorldStateModification& pEffect,
                                                        const std::vector<Parameter>& pParameters,
                                                        const Domain& pDomain,
                                                        FactsAlreadyChecked& pFactsAlreadychecked)
{
  std::set<Fact> accessibleFactsToAdd;
  std::vector<Fact> accessibleFactsToAddWithAnyValues;
  std::set<Fact> removableFactsToAdd;
  std::vector<Fact> removableFactsToAddWithAnyValues;

  const auto& setOfFacts = _worldState.factsMapping();
  pEffect.iterateOverAllAccessibleFacts([&](const ogp::FactOptional& pFactOpt) {
    if (!pFactOpt.isFactNegated)
    {
      if (_worldState.facts().count(pFactOpt.fact) == 0 &&
          _accessibleFacts.facts().count(pFactOpt.fact) == 0)
      {
        if (pFactOpt.fact.fluent() && pFactOpt.fact.fluent()->isAnyValue())
        {
          accessibleFactsToAddWithAnyValues.push_back(pFactOpt.fact);
        }
        else
        {
          auto factToInsert = pFactOpt.fact;
          if (factToInsert.replaceSomeArgumentsByAny(pParameters))
            accessibleFactsToAddWithAnyValues.push_back(std::move(factToInsert));
          else
            accessibleFactsToAdd.insert(std::move(factToInsert));
        }
      }
    }
    else
    {
      if (_removableFacts.facts().count(pFactOpt.fact) == 0)
      {
        if (pFactOpt.fact.fluent() && pFactOpt.fact.fluent()->isAnyValue())
        {
          removableFactsToAddWithAnyValues.push_back(pFactOpt.fact);
        }
        else
        {
          auto factToRemove = pFactOpt.fact;
          if (factToRemove.replaceSomeArgumentsByAny(pParameters))
            removableFactsToAddWithAnyValues.push_back(std::move(factToRemove));
          else
            removableFactsToAdd.insert(std::move(factToRemove));
        }
      }
    }
  }, setOfFacts);

  if (!accessibleFactsToAdd.empty() || !accessibleFactsToAddWithAnyValues.empty() ||
      !removableFactsToAdd.empty() || !removableFactsToAddWithAnyValues.empty())
  {
    for (auto& currFact : accessibleFactsToAdd)
      _accessibleFacts.add(currFact);
    _accessibleFactsWithAnyValues.insert(accessibleFactsToAddWithAnyValues.begin(), accessibleFactsToAddWithAnyValues.end());
    for (auto& currFact : removableFactsToAdd)
      _removableFacts.add(currFact);
    _removableFactsWithAnyValues.insert(removableFactsToAddWithAnyValues.begin(), removableFactsToAddWithAnyValues.end());
    for (const auto& currNewFact : accessibleFactsToAdd)
      _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked);
    for (const auto& currNewFact : accessibleFactsToAddWithAnyValues)
      _feedAccessibleFactsFromFact(currNewFact, pDomain, pFactsAlreadychecked);
    for (const auto& currNewFact : removableFactsToAdd)
      _feedAccessibleFactsFromNotFact(currNewFact, pDomain, pFactsAlreadychecked);
  }
}


void WorldStateCache::_feedAccessibleFactsFromFact(const Fact& pFact,
                                                   const Domain& pDomain,
                                                   FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.preconditionToActions().find(pFact);
  _feedAccessibleFactsFromSetOfActions(itPrecToActions, pDomain, pFactsAlreadychecked);

  const auto& setOfEvents = pDomain.getSetOfEvents();
  for (const auto& currSetOfEvents : setOfEvents)
  {
    auto& allEvents = currSetOfEvents.second.events();
    auto& conditionToReachableEvents = currSetOfEvents.second.reachableEventLinks().conditionToEvents;
    auto itCondToReachableEvents = conditionToReachableEvents.find(pFact);
    _feedAccessibleFactsFromSetOfEvents(itCondToReachableEvents, allEvents, pDomain, pFactsAlreadychecked);
  }
}


void WorldStateCache::_feedAccessibleFactsFromNotFact(const Fact& pFact,
                                                      const Domain& pDomain,
                                                      FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToRemove.insert(pFact).second)
    return;

  auto itPrecToActions = pDomain.notPreconditionToActions().find(pFact);
  _feedAccessibleFactsFromSetOfActions(itPrecToActions, pDomain, pFactsAlreadychecked);

  const auto& setOfEvents = pDomain.getSetOfEvents();
  for (const auto& currSetOfEvents : setOfEvents)
  {
    auto& allEvents = currSetOfEvents.second.events();
    auto& notconditionToReachableEvents = currSetOfEvents.second.reachableEventLinks().notConditionToEvents;
    auto itCondToReachableEvents = notconditionToReachableEvents.find(pFact);
    _feedAccessibleFactsFromSetOfEvents(itCondToReachableEvents, allEvents, pDomain, pFactsAlreadychecked);
  }
}


} // !ogp
