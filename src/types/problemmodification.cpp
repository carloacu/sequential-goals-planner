#include <contextualplanner/types/problemmodification.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{

bool ProblemModification::operator==(const ProblemModification& pOther) const
{
  return areUPtrEqual(worldStateModification, pOther.worldStateModification) &&
      areUPtrEqual(potentialWorldStateModification, pOther.potentialWorldStateModification) &&
      areUPtrEqual(worldStateModificationAtStart, pOther.worldStateModificationAtStart) &&
      goalsToAdd == pOther.goalsToAdd &&
      goalsToAddInCurrentPriority == pOther.goalsToAddInCurrentPriority;
}

} // !cp
