#include <contextualplanner/types/worldmodification.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{

bool WorldModification::operator==(const WorldModification& pOther) const
{
  return areUPtrEqual(factsModifications, pOther.factsModifications) &&
      areUPtrEqual(potentialFactsModifications, pOther.potentialFactsModifications) &&
      goalsToAdd == pOther.goalsToAdd &&
      goalsToAddInCurrentPriority == pOther.goalsToAddInCurrentPriority;
}

} // !cp
