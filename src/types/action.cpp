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


} // !cp
