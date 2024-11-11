#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP

#include <list>
#include <optional>
#include <set>
#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/goal.hpp>


namespace pgp
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
  void notifyNotSatisfiedGoal(const Goal& pGoal);
  std::size_t nbOfNotSatisfiedGoals() const { return _nbOfNonPersistentGoalsNotSatisfied; }
  std::size_t nbOfSatisfiedGoals() const { return _goalsSatisfied.size(); }
  bool isFirstGoalInSuccess() const { return _firstGoalInSuccess && *_firstGoalInSuccess; }
  void moveGoalsDone(std::list<Goal>& pGoals) { pGoals = std::move(_goalsSatisfied); }

private:
  PlannerStepType _type;
  std::size_t _nbOfNonPersistentGoalsNotSatisfied;
  std::list<Goal> _goalsSatisfied;
  std::set<const Goal*> _persistentGoalsSatisfied;
  std::optional<bool> _firstGoalInSuccess;
};

} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_LOOKFORANACTIONOUTPUTINFOS_HPP
