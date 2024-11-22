#include <orderedgoalsplanner/util/trackers/goalsremovedtracker.hpp>
#include <orderedgoalsplanner/types/goalstack.hpp>


namespace ogp
{


GoalsRemovedTracker::GoalsRemovedTracker(const GoalStack& pGoalStack)
  : _existingGoals(),
    _onGoalsChangedConnection(
      pGoalStack.onGoalsChanged.connectUnsafe([this](const std::map<int, std::vector<Goal>>& pGoals) {
  std::set<std::string> newGoals;
  for (const auto& currGoals : pGoals)
  {
    for (const auto& currGoal : currGoals.second)
    {
      auto goalStr = currGoal.toStr();
      _existingGoals.erase(goalStr);
      newGoals.insert(std::move(goalStr));
    }
  }

  if (!_existingGoals.empty())
    onGoalsRemoved(_existingGoals);
  _existingGoals = std::move(newGoals);
}))
{
}


GoalsRemovedTracker::~GoalsRemovedTracker()
{
  _onGoalsChangedConnection.disconnect();
}



} // !ogp

