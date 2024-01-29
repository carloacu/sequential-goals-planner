#include <contextualplanner/types/lookforanactionoutputinfos.hpp>


namespace cp
{

LookForAnActionOutputInfos::LookForAnActionOutputInfos()
 : _type(PlannerStepType::IN_PROGRESS),
   _nbOfNonPersistentGoalsNotSatisfied(0),
   _nbOfNonPersistentGoalsSatisfied(0),
   _persistentGoalsSatisfied(),
   _firstGoalInSuccess()
{
}

void LookForAnActionOutputInfos::notifySatisfiedGoal(const Goal& pGoal)
{
  if (!_firstGoalInSuccess)
    _firstGoalInSuccess.emplace(true);

  if (pGoal.isPersistent())
    _persistentGoalsSatisfied.insert(&pGoal);
  else
    ++_nbOfNonPersistentGoalsSatisfied;
}


void LookForAnActionOutputInfos::notifyNotSatisfiedGoal(const Goal& pGoal)
{
  if (!_firstGoalInSuccess)
    _firstGoalInSuccess.emplace(false);

  if (pGoal.isPersistent())
    _persistentGoalsSatisfied.erase(&pGoal);
  else
    ++_nbOfNonPersistentGoalsNotSatisfied;
}

} // !cp

