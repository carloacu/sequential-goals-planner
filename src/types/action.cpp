#include <contextualplanner/types/action.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{


bool Action::operator==(const Action& pOther) const
{
  return parameters == pOther.parameters &&
      areUPtrEqual(precondition, pOther.precondition) &&
      areUPtrEqual(preferInContext, pOther.preferInContext) &&
      effect == pOther.effect &&
      highImportanceOfNotRepeatingIt == pOther.highImportanceOfNotRepeatingIt;
}


bool Action::hasFact(const cp::Fact& pFact) const
{
  return (precondition && precondition->hasFact(pFact)) ||
      (preferInContext && preferInContext->hasFact(pFact)) ||
      effect.hasFact(pFact);
}

void Action::replaceArgument(const std::string& pOld,
                             const std::string& pNew)
{
  effect.replaceArgument(pOld, pNew);
}


void Action::limitPredicateTypes(const WorldState& pWorldState)
{
  _limitPredicateTypesForACondition(precondition);
  _limitPredicateTypesForACondition(preferInContext);
  _limitPredicateTypesForAnWordStateModif(effect.worldStateModification, pWorldState);
  _limitPredicateTypesForAnWordStateModif(effect.potentialWorldStateModification, pWorldState);
  _limitPredicateTypesForAnWordStateModif(effect.worldStateModificationAtStart, pWorldState);
}


void Action::_limitPredicateTypesForACondition(const std::unique_ptr<Condition>& pPrecondition)
{
  if (pPrecondition)
    pPrecondition->forAll([&](const FactOptional& pFactOptional) {
      _limitPredicateTypesForAFact(pFactOptional.fact);
    });
}


void Action::_limitPredicateTypesForAnWordStateModif(const std::unique_ptr<WorldStateModification>& pWs,
                                                     const WorldState& pWorldState)
{
  if (pWs)
    pWs->forAll([&](const FactOptional& pFactOptional) {
      _limitPredicateTypesForAFact(pFactOptional.fact);
    }, pWorldState);
}


void Action::_limitPredicateTypesForAFact(const Fact& pFact)
{
  for (auto& currArgument : pFact.arguments)
    if (currArgument.isAParameterToFill() && !currArgument.matchAny(parameters))
      throw std::runtime_error("\"" + currArgument.value + "\" is missing in action parameters");

  if (pFact.fluent && pFact.fluent->isAParameterToFill() && !pFact.fluent->matchAny(parameters))
    throw std::runtime_error("\"" + pFact.fluent->value + "\" fluent is missing in action parameters");
}



} // !cp
