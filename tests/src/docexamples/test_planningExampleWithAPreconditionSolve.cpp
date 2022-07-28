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

  // Action ids
  const std::string sayHi = "say_hi";
  const std::string askHowICanHelp = "ask_how_I_can_help";

  // Current clock to set to different functions
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  // Initialize the domain with a set of actions
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, {userIsGreeted}));
  actions.emplace(askHowICanHelp, cp::Action({userIsGreeted}, {proposedOurHelpToUser}));
  cp::Domain domain(actions);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.setGoals({proposedOurHelpToUser}, now);

  // Look for the next action to do
  std::map<std::string, std::string> parameters;
  auto actionToDo1 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert(sayHi == actionToDo1); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, actionToDo1, parameters, now);

  // Look for the next action to do
  auto actionToDo2 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert(askHowICanHelp == actionToDo2); // The action found is "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, actionToDo2, parameters, now);

  // Look for the next action to do
  auto actionToDo3 = cp::lookForAnActionToDo(parameters, problem, domain, now);
  assert("" == actionToDo3); // No action found
}

