#include <contextualplanner/types/problemmodification.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{

bool ProblemModification::operator==(const ProblemModification& pOther) const
{
  return areUPtrEqual(factsModifications, pOther.factsModifications) &&
      areUPtrEqual(potentialFactsModifications, pOther.potentialFactsModifications) &&
      goalsToAdd == pOther.goalsToAdd &&
      goalsToAddInCurrentPriority == pOther.goalsToAddInCurrentPriority;
}

} // !cp
