#include "test_planningDummyExample.hpp"
#include <map>
#include <memory>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>


void planningDummyExample()
{
  // Fact
  const std::string userIsGreeted = "user_is_greeted";

  // Action identifier
  const std::string sayHi = "say_hi";

  // Current clock to set to different functions
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  // Initialize the domain with an action
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, cp::WorldStateModification::fromStr(userIsGreeted, {}, {})));
  cp::Domain domain(actions);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.goalStack.setGoals({cp::Goal(userIsGreeted, {}, {})}, problem.worldState, now);

  // Look for an action to do
  auto planResult1 = cp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(!planResult1.empty());
  const auto& firstActionInPlan = planResult1.front();
  assert(sayHi == firstActionInPlan.actionInvocation.actionId); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, firstActionInPlan, now);

  // Look for the next action to do
  auto planResult2 = cp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(planResult2.empty()); // No action found
}

