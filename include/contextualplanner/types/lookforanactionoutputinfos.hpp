#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP

#include <list>
#include <set>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>


namespace cp
{

enum class PlannerStepType
{
  IN_PROGRESS,
  FINISHED_ON_SUCCESS,
  FINISEHD_ON_FAILURE
};


/// Output information from a plan resolution.
struct LookForAnActionOutputInfos
{
  LookForAnActionOutputInfos();

  void setType(PlannerStepType pType) { _type = pType; }
  PlannerStepType getType() const { return _type; }
  void notifySatisfiedGoal(const Goal& pGoal);
  std::size_t nbOfSatisfiedGoals() const { return _nbOfNonPersistentGoalsSatisfied + _persistentGoalsSatisfied.size(); }

private:
  PlannerStepType _type;
  std::size_t _nbOfNonPersistentGoalsSatisfied;
  std::set<const Goal*> _persistentGoalsSatisfied;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP
