#include "worldstatecache.hpp"
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include "factsalreadychecked.hpp"


namespace cp
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
                                      const std::set<Fact>& pFacts)
{
  if (_uuidOfLastDomainUsed == pDomain.getUuid())
    return;
  _uuidOfLastDomainUsed = pDomain.getUuid();

  for (int i = 0; i < 2; ++i) // 2 times to have all the accessible facts
  {
    FactsAlreadyChecked factsAlreadychecked;
    for (const auto& currFact : pFacts)
    {
      if (_accessibleFacts.facts().count(currFact) == 0)
      {
        auto itPrecToActions = pDomain.preconditionToActions().find(currFact);
        _feedAccessibleFactsFromSetOfActions(itPrecToActions, pDomain, factsAlreadychecked);
      }
    }
    auto actionWithoutPrecondition = pDomain.actionsWithoutFactToAddInPrecondition().valuesWithoutFact();
    _feedAccessibleFactsFromSetOfActions(actionWithoutPrecondition, pDomain,
                                         factsAlreadychecked);
  }
}


void WorldStateCache::_feedAccessibleFactsFromSetOfActions(const FactToConditions::ConstMapOfFactIterator& pActions,
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


void WorldStateCache::_feedAccessibleFactsFromSetOfInferences(const FactToConditions::ConstMapOfFactIterator& pInferences,
                                                              const std::map<InferenceId, Inference>& pAllInferences,
                                                              const Domain& pDomain,
                                                              FactsAlreadyChecked& pFactsAlreadychecked)
{
  for (const auto& currInference : pInferences)
  {
    auto itInference = pAllInferences.find(currInference);
    if (itInference != pAllInferences.end())
    {
      const Inference& inference = itInference->second;
      if (!inference.condition || inference.condition->canBecomeTrue(_worldState, inference.parameters))
        _feedAccessibleFactsFromDeduction(*inference.factsToModify, inference.parameters,
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

  pEffect.iterateOverAllAccessibleFacts([&](const cp::FactOptional& pFactOpt) {
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
  }, _worldState);

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

  const auto& setOfInferences = pDomain.getSetOfInferences();
  for (const auto& currSetOfInferences : setOfInferences)
  {
    auto& allInferences = currSetOfInferences.second.inferences();
    auto& conditionToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().conditionToInferences;
    auto itCondToReachableInferences = conditionToReachableInferences.find(pFact);
    _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences, allInferences, pDomain, pFactsAlreadychecked);
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

  const auto& setOfInferences = pDomain.getSetOfInferences();
  for (const auto& currSetOfInferences : setOfInferences)
  {
    auto& allInferences = currSetOfInferences.second.inferences();
    auto& notConditionToReachableInferences = currSetOfInferences.second.reachableInferenceLinks().notConditionToInferences;
    auto itCondToReachableInferences = notConditionToReachableInferences.find(pFact);
    _feedAccessibleFactsFromSetOfInferences(itCondToReachableInferences, allInferences, pDomain, pFactsAlreadychecked);
  }
}


} // !cp
