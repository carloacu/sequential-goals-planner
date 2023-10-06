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

void Action::replaceFact(const cp::Fact& pOldFact,
                         const cp::Fact& pNewFact)
{
  if (precondition)
    precondition->replaceFact(pOldFact, pNewFact);
  if (preferInContext)
    preferInContext->replaceFact(pOldFact, pNewFact);
  effect.replaceFact(pOldFact, pNewFact);
}


} // !cp
