#ifndef INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
#define INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP

#include <set>
#include <string>
#include "../api.hpp"
#include <prioritizedgoalsplanner/util/observableunsafe.hpp>


namespace pgp
{
struct GoalStack;

struct PRIORITIZEDGOALSPLANNER_API GoalsRemovedTracker
{
  GoalsRemovedTracker(const GoalStack& pGoalStack);
  ~GoalsRemovedTracker();
  pgpstd::observable::ObservableUnsafe<void (const std::set<std::string>&)> onGoalsRemoved{};


private:
  std::set<std::string> _existingGoals;
  pgpstd::observable::Connection _onGoalsChangedConnection;
};

} // !pgp


#endif // INCLUDE_TRACKERS_GOALSREMOVEDTRACKER_HPP
