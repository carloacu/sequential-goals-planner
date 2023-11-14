#include "test_planningExampleWithAPreconditionSolve.hpp"
#include <map>
#include <memory>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>


void planningExampleWithAPreconditionSolve()
{
  // Facts
  const std::string userIsGreeted = "user_is_greeted";
  const std::string proposedOurHelpToUser = "proposed_our_help_to_user";

  // Action identifiers
  const std::string sayHi = "say_hi";
  const std::string askHowICanHelp = "ask_how_I_can_help";

  // Current clock to set to different functions
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  // Initialize the domain with a set of actions
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, cp::WorldStateModification::fromStr(userIsGreeted)));
  actions.emplace(askHowICanHelp, cp::Action(cp::Condition::fromStr(userIsGreeted),
                                             cp::WorldStateModification::fromStr(proposedOurHelpToUser)));
  cp::Domain domain(actions);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.goalStack.setGoals({proposedOurHelpToUser}, problem.worldState, now);

  // Look for an action to do
  auto oneStepOfPlannerResult1 = cp::lookForAnActionToDo(problem, domain, now);
  assert(oneStepOfPlannerResult1.operator bool());
  assert(sayHi == oneStepOfPlannerResult1->actionInstance.actionId); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, *oneStepOfPlannerResult1, now);

  // Look for the next action to do
  auto oneStepOfPlannerResult2 = cp::lookForAnActionToDo(problem, domain, now);
  assert(oneStepOfPlannerResult2.operator bool());
  assert(askHowICanHelp == oneStepOfPlannerResult2->actionInstance.actionId); // The action found is "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, *oneStepOfPlannerResult2, now);

  // Look for the next action to do
  auto oneStepOfPlannerResult3 = cp::lookForAnActionToDo(problem, domain, now);
  assert(!oneStepOfPlannerResult3.operator bool()); // No action found
}

