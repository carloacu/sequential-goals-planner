#include <contextualplanner/types/lookforanactionoutputinfos.hpp>


namespace cp
{

LookForAnActionOutputInfos::LookForAnActionOutputInfos()
 : _type(PlannerStepType::IN_PROGRESS),
   _nbOfNonPersistentGoalsNotSatisfied(0),
   _goalsSatisfied(),
   _persistentGoalsSatisfied(),
   _firstGoalInSuccess()
{
}

void LookForAnActionOutputInfos::notifySatisfiedGoal(const Goal& pGoal)
{
  if (!_firstGoalInSuccess)
    _firstGoalInSuccess.emplace(true);

  if (pGoal.isPersistent())
  {
    if (!_persistentGoalsSatisfied.insert(&pGoal).second)
      return; // Skip because this persistent goal has already been seen
  }
  _goalsSatisfied.emplace_back(pGoal);
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

