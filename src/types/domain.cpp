#include <contextualplanner/types/domain.hpp>


namespace cp
{

Domain::Domain(const std::map<ActionId, Action>& pActions)
{
  for (const auto& currAction : pActions)
    addAction(currAction.first, currAction.second);
}


void Domain::addAction(const ActionId& pActionId,
                       const Action& pAction)
{
  if ((pAction.effect.factsModifications.isIncludedIn(pAction.precondition) &&
       pAction.effect.potentialFactsModifications.isIncludedIn(pAction.precondition)) ||
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
    },
    [&](const Expression& pExpression)
    {
      for (auto& currElt : pExpression.elts)
      {
        if (currElt.type == ExpressionElementType::FACT)
        {
          _preconditionToActionsExps[currElt.value].insert(pActionId);
          hasAddedAFact = true;
        }
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
    },
    [&](const Expression& pExpression)
    {
      for (auto& currElt : pExpression.elts)
        if (currElt.type == ExpressionElementType::FACT)
          _preconditionToActionsExps[currElt.value].erase(pActionId);
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
