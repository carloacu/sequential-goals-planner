#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/prioritizedgoalsplanner.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>

namespace
{
const std::map<pgp::SetOfEventsId, pgp::SetOfEvents> _emptySetOfEvents;
const std::string _sep = ", ";
const std::unique_ptr<std::chrono::steady_clock::time_point> _now = {};

const std::string _fact_a = "fact_a";
const std::string _fact_b = "fact_b";
const std::string _fact_c = "fact_c";
const std::string _fact_d = "fact_d";
const std::string _fact_e = "fact_e";
const std::string _fact_f = "fact_f";
const std::string _fact_g = "fact_g";


static const std::vector<pgp::Parameter> _emptyParameters;


pgp::Fact _fact(const std::string& pStr,
               const pgp::Ontology& pOntology,
               const std::vector<pgp::Parameter>& pParameters = {}) {
  return pgp::Fact(pStr, false, pOntology, {}, pParameters);
}

pgp::Parameter _parameter(const std::string& pStr,
                         const pgp::Ontology& pOntology) {
  return pgp::Parameter::fromStr(pStr, pOntology.types);
}

pgp::Goal _goal(const std::string& pStr,
               const pgp::Ontology& pOntology,
               int pMaxTimeToKeepInactive = -1,
               const std::string& pGoalGroupId = "") {
  return pgp::Goal::fromStr(pStr, pOntology, {}, pMaxTimeToKeepInactive, pGoalGroupId);
}

std::unique_ptr<pgp::Condition> _condition_fromStr(const std::string& pConditionStr,
                                                  const pgp::Ontology& pOntology,
                                                  const std::vector<pgp::Parameter>& pParameters = {}) {
  return pgp::strToCondition(pConditionStr, pOntology, {}, pParameters);
}

std::unique_ptr<pgp::WorldStateModification> _worldStateModification_fromStr(const std::string& pStr,
                                                                            const pgp::Ontology& pOntology,
                                                                            const std::vector<pgp::Parameter>& pParameters = {}) {
  return pgp::strToWsModification(pStr, pOntology, {}, pParameters);
}

void _setGoalsForAPriority(pgp::Problem& pProblem,
                           const std::vector<pgp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = pgp::GoalStack::defaultPriority)
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow, pPriority);
}


void _addFact(pgp::WorldState& pWorldState,
              const std::string& pFactStr,
              pgp::GoalStack& pGoalStack,
              const pgp::Ontology& pOntology,
              const std::map<pgp::SetOfEventsId, pgp::SetOfEvents>& pSetOfEvents = _emptySetOfEvents,
              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {}) {
  pWorldState.addFact(_fact(pFactStr, pOntology), pGoalStack, pSetOfEvents, pOntology, pgp::SetOfEntities(), pNow);
}

pgp::ActionInvocationWithGoal _lookForAnActionToDo(pgp::Problem& pProblem,
                                                  const pgp::Domain& pDomain,
                                                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                                  const pgp::Historical* pGlobalHistorical = nullptr)
{
  auto plan = pgp::planForMoreImportantGoalPossible(pProblem, pDomain, true, pNow, pGlobalHistorical);
  if (!plan.empty())
    return plan.front();
  return pgp::ActionInvocationWithGoal("", std::map<pgp::Parameter, pgp::Entity>(), {}, 0);
}


std::string _lookForAnActionToDoInParallelThenNotifyToStr(
    pgp::Problem& pProblem,
    const pgp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  auto actionsToDoInParallel = pgp::actionsToDoInParallelNow(pProblem, pDomain, pNow);
  for (auto& currAction : actionsToDoInParallel.actions)
  {
    notifyActionStarted(pProblem, pDomain, currAction, pNow);
    notifyActionDone(pProblem, pDomain, currAction, pNow);
  }
  return pgp::planToStr(actionsToDoInParallel.actions);
}

std::string _parallelPlanStr(pgp::Problem& pProblem,
                             const pgp::Domain& pDomain,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                             pgp::Historical* pGlobalHistorical = nullptr)
{
  return pgp::parallelPlanToStr(pgp::parallelPlanForEveryGoals(pProblem, pDomain, pNow, pGlobalHistorical));
}

std::string _parallelPlanPddl(pgp::Problem& pProblem,
                              const pgp::Domain& pDomain,
                              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                              pgp::Historical* pGlobalHistorical = nullptr)
{
  return pgp::parallelPlanToPddl(pgp::parallelPlanForEveryGoals(pProblem, pDomain, pNow, pGlobalHistorical), pDomain);
}


void _goalsToDoInParallel()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";
  const std::string action5 = "action5";
  const std::string action6 = "action6";
  const std::string action7 = "action7";

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("val1 val2 val3 val4 - entity", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "\n" +
                                                     _fact_f + "\n" +
                                                     _fact_g, ontology.types);

  std::map<std::string, pgp::Action> actions;
  std::vector<pgp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  pgp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  pgp::Action actionObj2(_condition_fromStr(_fact_a + "=val1", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action2, actionObj2);

  pgp::Action actionObj3(_condition_fromStr(_fact_a + "=val2", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action3, actionObj3);

  pgp::Action actionObj4(_condition_fromStr(_fact_a + "=val3 & !" + _fact_c, ontology),
                        _worldStateModification_fromStr("!" + _fact_d + " & " + _fact_f, ontology));
  actions.emplace(action4, actionObj4);

  pgp::Action actionObj5(_condition_fromStr(_fact_a + "=val4", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action5, actionObj5);

  pgp::Action actionObj6(_condition_fromStr(_fact_b + " & !" + _fact_d + " & " + _fact_g, ontology),
                        _worldStateModification_fromStr(_fact_e, ontology));
  actions.emplace(action6, actionObj6);

  pgp::Action actionObj7(_condition_fromStr(_fact_f, ontology),
                        _worldStateModification_fromStr(_fact_g, ontology));
  actions.emplace(action7, actionObj7);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal(_fact_e, ontology)});

  EXPECT_EQ(action1 + "(?obj -> val3)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());

  EXPECT_EQ(action1 + "(?obj -> val3)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  EXPECT_EQ(action4, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  EXPECT_EQ(action7 + _sep + action1 + "(?obj -> val1)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  EXPECT_EQ(action2, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  EXPECT_EQ(action6, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  EXPECT_EQ("", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
}

void _2goalsInParallel()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                      _fact_b, ontology.types);

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));
  actions.emplace(action2, pgp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));

  pgp::Domain domain(std::move(actions), ontology);
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a, ontology), _goal(_fact_b, ontology)});
  EXPECT_EQ("action1, action2", _parallelPlanStr(problem, domain, _now));
}


void _goalsToDoInParallelWith2Goals()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";

  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                      _fact_b + "\n" +
                                                      _fact_c + "\n" +
                                                      _fact_d, ontology.types);

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));
  actions.emplace(action2, pgp::Action(_condition_fromStr(_fact_a, ontology),
                                       _worldStateModification_fromStr(_fact_b, ontology)));
  actions.emplace(action3, pgp::Action({}, _worldStateModification_fromStr(_fact_c, ontology)));
  actions.emplace(action4, pgp::Action({}, _worldStateModification_fromStr(_fact_d, ontology)));

  pgp::Domain domain(std::move(actions), ontology);
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a, ontology), _goal(_fact_b + " & " + _fact_c + " & " + _fact_d, ontology)});
  auto problem2 = problem;

  EXPECT_EQ("action1, action3, action4\n"
            "action2", _parallelPlanStr(problem, domain, _now));

  EXPECT_EQ("00: (action1) [1]\n"
            "00: (action3) [1]\n"
            "00: (action4) [1]\n"
            "01: (action2) [1]\n", _parallelPlanPddl(problem2, domain, _now));
}


void _goalsToDoInParallelWithConflitingEffects()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("v1 v2 - entity", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                      _fact_b + "\n" +
                                                      _fact_c, ontology.types);

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action({}, _worldStateModification_fromStr(_fact_a + "=v1 & " + _fact_b, ontology)));
  actions.emplace(action2, pgp::Action({}, _worldStateModification_fromStr(_fact_a + "=v2 & " + _fact_c, ontology)));

  pgp::Domain domain(std::move(actions), ontology);
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology), _goal(_fact_c, ontology)});

  EXPECT_EQ("action1\n"
            "action2", _parallelPlanStr(problem, domain, _now));
}


}



TEST(Planner, test_parallelPlan)
{
  _goalsToDoInParallel();
  _2goalsInParallel();
  _goalsToDoInParallelWith2Goals();
  _goalsToDoInParallelWithConflitingEffects();
}