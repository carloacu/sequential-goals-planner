#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/util/util.hpp>
#include "../util/uuid.hpp"

namespace cp
{
const SetOfEventsId Domain::setOfEventsIdFromConstructor = "soe_from_constructor";

namespace
{
static const WorldState _emptyWorldState;
static const std::map<Parameter, std::set<Entity>> _emptyParametersWithValues;
static const std::vector<Parameter> _emptyParameters;

/**
 * @brief Check if a world state modification can do some modification if we assume the world already satisfies a condition.
 * @param[in] pWorldStateModification World state modification to check.
 * @param[in] pSatisfiedConditionPtr Condition that is already satisfied.
 * @return True if the world state modification can do some modification in the world.
 */
bool _canWmDoSomething(const std::unique_ptr<cp::WorldStateModification>& pWorldStateModification,
                       const std::unique_ptr<Condition>& pSatisfiedConditionPtr)
{
  if (!pWorldStateModification)
    return false;
  if (!pWorldStateModification->isOnlyASetOfFacts())
    return true;

  if (pWorldStateModification->forAllUntilTrue(
        [&](const FactOptional& pFactOptional)
  {
      return !pSatisfiedConditionPtr ||
        !pSatisfiedConditionPtr->containsFactOpt(pFactOptional,
                                                 _emptyParametersWithValues, nullptr,
                                                 _emptyParameters);
}, _emptyWorldState))
  {
    return true;
  }

  return false;
}

}

Domain::Domain()
  : _uuid(),
    _ontology(),
    _actions(),
    _preconditionToActions(),
    _notPreconditionToActions(),
    _actionsWithoutFactToAddInPrecondition(),
    _setOfEvents()
{
}


Domain::Domain(const std::map<ActionId, Action>& pActions,
               const Ontology& pOntology,
               const SetOfEvents& pSetOfEvents)
  : _uuid(generateUuid()),
    _ontology(pOntology),
    _actions(),
    _preconditionToActions(),
    _notPreconditionToActions(),
    _actionsWithoutFactToAddInPrecondition(),
    _setOfEvents()
{
  for (const auto& currAction : pActions)
    addAction(currAction.first, currAction.second);

  if (!pSetOfEvents.empty())
    _setOfEvents.emplace(setOfEventsIdFromConstructor, pSetOfEvents);

  _updateSuccessions();
}


void Domain::addAction(const ActionId& pActionId,
                       const Action& pAction)
{
  if (_actions.count(pActionId) > 0 ||
      pAction.effect.empty())
    return;
  const auto& action = _actions.emplace(pActionId, pAction.clone(_ontology.derivedPredicates)).first->second;

  if (!_canWmDoSomething(action.effect.worldStateModification, action.precondition) &&
      !_canWmDoSomething(action.effect.potentialWorldStateModification, action.precondition))
    return;

  _uuid = generateUuid(); // Regenerate uuid to force the problem to refresh his cache when it will use this object

  bool hasAddedAFact = false;
  if (action.precondition)
  {
    action.precondition->forAll(
          [&](const FactOptional& pFactOptional,
              bool pIgnoreFluent)
    {
      if (pFactOptional.isFactNegated)
      {
        _notPreconditionToActions.add(pFactOptional.fact, pActionId, pIgnoreFluent);
      }
      else
      {
        _preconditionToActions.add(pFactOptional.fact, pActionId, pIgnoreFluent);
        hasAddedAFact = true;
      }
    }
    );
  }

  if (!hasAddedAFact)
    _actionsWithoutFactToAddInPrecondition.addValueWithoutFact(pActionId);
  _updateSuccessions();
}


void Domain::removeAction(const ActionId& pActionId)
{
  auto it = _actions.find(pActionId);
  if (it == _actions.end())
    return;
  auto& actionThatWillBeRemoved = it->second;
  _uuid = generateUuid(); // Regenerate uuid to force the problem to refresh his cache when it will use this object

  if (actionThatWillBeRemoved.precondition)
  {
    _notPreconditionToActions.erase(pActionId);
    _preconditionToActions.erase(pActionId);
  }
  else
  {
    _actionsWithoutFactToAddInPrecondition.erase(pActionId);
  }

  _actions.erase(it);
  _updateSuccessions();
}



SetOfEventsId Domain::addSetOfEvents(const SetOfEvents& pSetOfEvents,
                                             const SetOfEventsId& pSetOfEventsId)
{
  _uuid = generateUuid(); // Regenerate uuid to force the problem to refresh his cache when it will use this object
  auto isIdOkForInsertion = [this](const std::string& pId)
  {
    return _setOfEvents.count(pId) == 0;
  };

  auto newId = incrementLastNumberUntilAConditionIsSatisfied(pSetOfEventsId, isIdOkForInsertion);
  _setOfEvents.emplace(newId, pSetOfEvents);
  _updateSuccessions();
  return newId;
}


void Domain::removeSetOfEvents(const SetOfEventsId& pSetOfEventsId)
{
  auto it = _setOfEvents.find(pSetOfEventsId);
  if (it != _setOfEvents.end())
  {
    _uuid = generateUuid(); // Regenerate uuid to force the problem to refresh his cache when it will use this object
    _setOfEvents.erase(it);
    _updateSuccessions();
  }
}

void Domain::clearEvents()
{
  if (!_setOfEvents.empty())
  {
    _uuid = generateUuid(); // Regenerate uuid to force the problem to refresh his cache when it will use this object
    _setOfEvents.clear();
    _updateSuccessions();
  }
}


std::string Domain::printSuccessionCache() const
{
  std::string res;
   for (const auto& currAction : _actions)
   {
     const Action& action = currAction.second;
     auto sc = action.printSuccessionCache();
     if (!sc.empty())
     {
       if (!res.empty())
         res += "\n\n";
       res += "action: " + currAction.first + "\n";
       res += "----------------------------------\n\n";
       res += sc;
     }
   }

   for (const auto& currSetOfEv : _setOfEvents)
   {
     for (const auto& currEv : currSetOfEv.second.events())
     {
       const Event& event = currEv.second;
       auto sc = event.printSuccessionCache();
       if (!sc.empty())
       {
         if (!res.empty())
           res += "\n\n";
         res += "event: " + currSetOfEv.first + "|" + currEv.first + "\n";
         res += "----------------------------------\n\n";
         res += sc;
       }
     }
   }

   return res;
}


void Domain::_updateSuccessions()
{
  for (auto& currAction: _actions)
    currAction.second.updateSuccessionCache(*this, currAction.first);

  for (auto& currSetOfEvents : _setOfEvents)
  {
    const auto& currSetOfEventsId = currSetOfEvents.first;
    for (auto& currEvent : currSetOfEvents.second.events())
      currEvent.second.updateSuccessionCache(*this, currSetOfEventsId, currEvent.first);
  }
}



} // !cp
