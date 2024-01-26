#include <contextualplanner/types/lookforanactionoutputinfos.hpp>


namespace cp
{

LookForAnActionOutputInfos::LookForAnActionOutputInfos()
 : _type(PlannerStepType::IN_PROGRESS),
   _nbOfNonPersistentGoalsNotSatisfied(0),
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


void LookForAnActionOutputInfos::notifyNotSatisfiedGoal(const Goal& pGoal)
{
  if (pGoal.isPersistent())
    _persistentGoalsSatisfied.erase(&pGoal);
  else
    ++_nbOfNonPersistentGoalsNotSatisfied;
}

} // !cp

