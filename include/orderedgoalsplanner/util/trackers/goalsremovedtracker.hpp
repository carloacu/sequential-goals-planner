#ifndef INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
#define INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP

#include <set>
#include <string>
#include "../api.hpp"
#include <orderedgoalsplanner/util/observableunsafe.hpp>


namespace ogp
{
struct GoalStack;

struct ORDEREDGOALSPLANNER_API GoalsRemovedTracker
{
  GoalsRemovedTracker(const GoalStack& pGoalStack);
  ~GoalsRemovedTracker();
  ogpstd::observable::ObservableUnsafe<void (const std::set<std::string>&)> onGoalsRemoved{};


private:
  std::set<std::string> _existingGoals;
  ogpstd::observable::Connection _onGoalsChangedConnection;
};

} // !ogp


#endif // INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
