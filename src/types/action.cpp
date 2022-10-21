#include <contextualplanner/types/action.hpp>


namespace cp
{


bool Action::hasFact(const cp::Fact& pFact) const
{
  return preconditions.hasFact(pFact) ||
      preferInContext.hasFact(pFact) ||
      effect.hasFact(pFact);
}

void Action::replaceFact(const cp::Fact& pOldFact,
                         const cp::Fact& pNewFact)
{
  preconditions.replaceFact(pOldFact, pNewFact);
  preferInContext.replaceFact(pOldFact, pNewFact);
  effect.replaceFact(pOldFact, pNewFact);
}


} // !cp
