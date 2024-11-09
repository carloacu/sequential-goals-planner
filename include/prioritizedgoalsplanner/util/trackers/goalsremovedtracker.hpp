#ifndef INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
#define INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP

#include <set>
#include <string>
#include "../api.hpp"
#include <prioritizedgoalsplanner/util/observableunsafe.hpp>


namespace cp
{
struct GoalStack;

struct CONTEXTUALPLANNER_API GoalsRemovedTracker
{
  GoalsRemovedTracker(const GoalStack& pGoalStack);
  ~GoalsRemovedTracker();
  cpstd::observable::ObservableUnsafe<void (const std::set<std::string>&)> onGoalsRemoved{};


private:
  std::set<std::string> _existingGoals;
  cpstd::observable::Connection _onGoalsChangedConnection;
};

} // !cp


#endif // INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
