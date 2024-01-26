#include <contextualplanner/types/lookforanactionoutputinfos.hpp>


namespace cp
{

LookForAnActionOutputInfos::LookForAnActionOutputInfos()
 : _type(PlannerStepType::IN_PROGRESS),
   _nbOfNonPersistentGoalsSatisfied(0),
   _persistentGoalsSatisfied()
{
}

void LookForAnActionOutputInfos::notifySatisfiedGoal(const Goal& pGoal)
{
  if (pGoal.isPersistent())
    _persistentGoalsSatisfied.insert(&pGoal);
  else
    ++_nbOfNonPersistentGoalsSatisfied;
}


} // !cp

