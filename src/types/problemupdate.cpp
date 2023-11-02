#include <contextualplanner/types/problemupdate.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{

bool ProblemUpdate::operator==(const ProblemUpdate& pOther) const
{
  return areUPtrEqual(factsModifications, pOther.factsModifications) &&
      areUPtrEqual(potentialFactsModifications, pOther.potentialFactsModifications) &&
      goalsToAdd == pOther.goalsToAdd &&
      goalsToAddInCurrentPriority == pOther.goalsToAddInCurrentPriority;
}

} // !cp
