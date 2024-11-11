#include "test_planningExampleWithAPreconditionSolve.hpp"
#include <map>
#include <memory>
#include <assert.h>
#include <prioritizedgoalsplanner/prioritizedgoalsplanner.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>


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

  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr(userIsGreeted + "\n" +
                                                     proposedOurHelpToUser, ontology.types);

  // Initialize the domain with a set of actions
  std::map<pgp::ActionId, pgp::Action> actions;
  actions.emplace(sayHi, pgp::Action({}, pgp::strToWsModification(userIsGreeted, ontology, {}, {})));
  actions.emplace(askHowICanHelp, pgp::Action(pgp::strToCondition(userIsGreeted, ontology, {}, {}),
                                             pgp::strToWsModification(proposedOurHelpToUser, ontology, {}, {})));
  pgp::Domain domain(actions, ontology);

  // Initialize the problem with the goal to satisfy
  pgp::Problem problem;
  problem.goalStack.setGoals({pgp::Goal::fromStr(proposedOurHelpToUser, ontology, {})}, problem.worldState, now);

  // Look for an action to do
  auto planResult1 = pgp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(!planResult1.empty());
  const auto& firstActionInPlan1 = planResult1.front();
  assert(sayHi == firstActionInPlan1.actionInvocation.actionId); // The action found is "say_hi", because it is needed to satisfy the preconditions of "ask_how_I_can_help"
  // When the action is finished we notify the planner
  pgp::notifyActionDone(problem, domain, firstActionInPlan1, now);

  // Look for the next action to do
  auto planResult2 = pgp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(!planResult2.empty());
  const auto& firstActionInPlan2 = planResult2.front();
  assert(askHowICanHelp == firstActionInPlan2.actionInvocation.actionId); // The action found is "ask_how_I_can_help"
  // When the action is finished we notify the planner
  pgp::notifyActionDone(problem, domain, firstActionInPlan2, now);

  // Look for the next action to do
  auto planResult3 = pgp::planForMoreImportantGoalPossible(problem, domain, true, now);
  assert(planResult3.empty()); // No action found
}

