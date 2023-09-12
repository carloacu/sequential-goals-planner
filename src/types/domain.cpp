#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/problem.hpp>

namespace cp
{
namespace
{
static const Problem _emptyProblem;
static const std::map<std::string, std::set<std::string>> _emptyParametersWithValues;
static const std::vector<std::string> _emptyParameters;

/**
 * @brief Check if this object is totally included in another SetOfFacts object.
 * @param pOther The other SetOfFacts to check.
 * @return True if this object is included, false otherwise.
 */
bool _isIncludedIn(const std::unique_ptr<cp::FactModification>& pFactsModifications,
                   const std::unique_ptr<FactCondition>& pFactConditionPtr)
{
  if (!pFactsModifications)
    return true;
  if (pFactsModifications->isDynamic())
    return false;

  if (pFactsModifications->forAllUntilTrue(
        [&](const FactOptional& pFactOptional)
  {
    return !pFactConditionPtr || !pFactConditionPtr->containsFactOpt(pFactOptional,
                                                                     _emptyParametersWithValues,
                                                                     _emptyParameters);
  }, _emptyProblem))
  {
    return false;
  }

  return true;
}

}


Domain::Domain(const std::map<ActionId, Action>& pActions)
{
  for (const auto& currAction : pActions)
    addAction(currAction.first, currAction.second);
}


void Domain::addAction(const ActionId& pActionId,
                       const Action& pAction)
{
  if ((_isIncludedIn(pAction.effect.factsModifications, pAction.precondition) &&
       _isIncludedIn(pAction.effect.potentialFactsModifications, pAction.precondition)) ||
      pAction.effect.empty() ||
      _actions.count(pActionId) > 0)
    return;
  _actions.emplace(pActionId, pAction);

  bool hasAddedAFact = false;
  if (pAction.precondition)
  {
    pAction.precondition->forAll(
          [&](const FactOptional& pFactOptional)
    {
      if (pFactOptional.isFactNegated)
      {
        _notPreconditionToActions[pFactOptional.fact.name].insert(pActionId);
      }
      else
      {
        _preconditionToActions[pFactOptional.fact.name].insert(pActionId);
        hasAddedAFact = true;
      }
    }
    );
  }

  if (!hasAddedAFact)
    _actionsWithoutFactToAddInPrecondition.insert(pActionId);
}


void Domain::removeAction(const ActionId& pActionId)
{
  auto it = _actions.find(pActionId);
  if (it == _actions.end())
    return;
  auto& actionThatWillBeRemoved = it->second;

  if (actionThatWillBeRemoved.precondition)
  {
    actionThatWillBeRemoved.precondition->forAll(
          [&](const FactOptional& pFactOptional)
    {
      if (pFactOptional.isFactNegated)
        _notPreconditionToActions[pFactOptional.fact.name].erase(pActionId);
      else
        _preconditionToActions[pFactOptional.fact.name].erase(pActionId);
    }
    );
  }
  else
  {
     _actionsWithoutFactToAddInPrecondition.erase(pActionId);
  }

  _actions.erase(it);
}


} // !cp
