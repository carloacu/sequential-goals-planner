#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/types/domain.hpp>
#include <prioritizedgoalsplanner/types/ontology.hpp>
#include <prioritizedgoalsplanner/types/problem.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>


using namespace pgp;

void _setGoalsForAPriority(pgp::Problem& pProblem,
                           const std::vector<pgp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = pgp::GoalStack::defaultPriority)
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow, pPriority);
}

std::string _actionIdsToStr(const std::set<ActionId>& pActionIds)
{
  std::ostringstream oss;
  for (auto it = pActionIds.begin(); it != pActionIds.end(); ++it) {
      if (it != pActionIds.begin()) {
          oss << ", ";
      }
      oss << *it;
  }
  return oss.str();
}



TEST(Tool, test_goalsCache)
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("e1 e2 - entity\n"
                                            "result_type");
  ontology.constants = pgp::SetOfEntities::fromPddl("a b - entity\n"
                                                   "r1 r2 - result_type", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("fact_a\n"
                                                     "fact_b(?e - entity) - result_type\n"
                                                     "fact_c\n"
                                                     "fact_d\n"
                                                     "fact_e(?e - entity) - result_type\n"
                                                     "fact_f(?e - entity)\n"
                                                     "fact_h",
                                                     ontology.types);

  std::map<std::string, pgp::Action> actions;

  {
    std::vector<pgp::Parameter> parameters1(1, pgp::Parameter::fromStr("?e - e1", ontology.types));
    pgp::Action actionObj1(pgp::strToCondition("fact_d", ontology, {}, parameters1),
                          pgp::strToWsModification("fact_d & not(fact_a) & assign(fact_b(?e), r1)", ontology, {}, parameters1));
    actionObj1.parameters = std::move(parameters1);
    actions.emplace(action1, actionObj1);
  }

  {
    pgp::Action actionObj2(pgp::strToCondition("fact_c", ontology, {}, {}),
                          pgp::strToWsModification("fact_d", ontology, {}, {}));
    actions.emplace(action2, actionObj2);
  }

  {
    pgp::Action actionObj3({},
                          pgp::strToWsModification("fact_c", ontology, {}, {}));
    actions.emplace(action3, actionObj3);
  }

  {
    pgp::Action actionObj4(pgp::strToCondition("fact_h", ontology, {}, {}),
                          pgp::strToWsModification("fact_a", ontology, {}, {}));
    actions.emplace(action4, actionObj4);
  }

  SetOfEvents setOfEvents;
  std::vector<Parameter> eventParameters{Parameter::fromStr("?e - entity", ontology.types)};
  {
    pgp::Event event(pgp::strToCondition("fact_a", ontology, {}, eventParameters),
                    pgp::strToWsModification("forall(?e - entity, when(fact_f(?e), set(fact_b(?e), fact_e(?e))))", ontology, {}, eventParameters));
    event.parameters = std::move(eventParameters);
    setOfEvents.add(event);
  }
  {
    pgp::Event event(pgp::strToCondition("fact_d", ontology, {}, eventParameters),
                    pgp::strToWsModification("fact_h", ontology, {}, eventParameters));
    event.parameters = std::move(eventParameters);
    setOfEvents.add(event);
  }
  Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  const auto& domainOntology = domain.getOntology();

  Problem problem;
  auto& entities = problem.entities;
  entities = pgp::SetOfEntities::fromPddl("ent - entity\n"
                                         "sub_ent1 - e1\n"
                                         "sub_ent2 - e2", ontology.types);

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("fact_d", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: fact_d\n"
              "---------------------------\n"
              "actions: action2", problem.goalStack.printGoalsCache());
    EXPECT_EQ("action2, action3", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("=(fact_b(ent), r2)", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: fact_b(ent)=r2\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
    EXPECT_EQ("action2, action3, action4", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("soe_from_constructor|event, soe_from_constructor|event_2", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("!fact_c", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("", problem.goalStack.printGoalsCache());
    EXPECT_EQ("", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("not(=(fact_b(ent), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(ent)=r1\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
    EXPECT_EQ("action2, action3, action4", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("soe_from_constructor|event, soe_from_constructor|event_2", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("not(=(fact_b(sub_ent1), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent1)=r1\n"
              "---------------------------\n"
              "actions: action1\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
    EXPECT_EQ("action1, action2, action3, action4", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("soe_from_constructor|event, soe_from_constructor|event_2", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("not(=(fact_b(sub_ent2), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent2)=r1\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
    EXPECT_EQ("action2, action3, action4", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("soe_from_constructor|event, soe_from_constructor|event_2", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }

  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("not(=(fact_b(sub_ent1), r2))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent1)=r2\n"
              "---------------------------\n"
              "actions: action1\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
    EXPECT_EQ( "action1, action2, action3, action4", _actionIdsToStr(problem.goalStack.getActionsPredecessors()));
    EXPECT_EQ("soe_from_constructor|event, soe_from_constructor|event_2", _actionIdsToStr(problem.goalStack.getEventsPredecessors()));
  }
}

