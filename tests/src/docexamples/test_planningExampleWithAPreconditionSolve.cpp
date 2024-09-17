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

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(userIsGreeted + "\n" +
                                                     proposedOurHelpToUser, ontology.types);

  // Initialize the domain with a set of actions
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(sayHi, cp::Action({}, cp::WorldStateModification::fromStr(userIsGreeted, ontology, {}, {})));
  actions.emplace(askHowICanHelp, cp::Action(cp::Condition::fromStr(userIsGreeted, ontology, {}, {}),
                                             cp::WorldStateModification::fromStr(proposedOurHelpToUser, ontology, {}, {})));
  cp::Domain domain(actions, ontology);

  // Initialize the problem with the goal to satisfy
  cp::Problem problem;
  problem.goalStack.setGoals({cp::Goal(proposedOurHelpToUser, ontology, {})}, problem.worldState, now);

  // Look for an action to do
  auto planResult1 = cp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(!planResult1.empty());
  const auto& firstActionInPlan1 = planResult1.front();
  assert(sayHi == firstActionInPlan1.actionInvocation.actionId); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, firstActionInPlan1, now);

  // Look for the next action to do
  auto planResult2 = cp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(!planResult2.empty());
  const auto& firstActionInPlan2 = planResult2.front();
  assert(askHowICanHelp == firstActionInPlan2.actionInvocation.actionId); // The action found is "ask_how_I_can_help"
  // When the action is finished we notify the planner
  cp::notifyActionDone(problem, domain, firstActionInPlan2, now);

  // Look for the next action to do
  auto planResult3 = cp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(planResult3.empty()); // No action found
}

