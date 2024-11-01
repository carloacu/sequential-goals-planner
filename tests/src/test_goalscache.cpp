#include <gtest/gtest.h>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/ontology.hpp>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/util/serializer/deserializefrompddl.hpp>


using namespace cp;

void _setGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<cp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow, pPriority);
}

TEST(Tool, test_goalsCache)
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromPddl("e1 e2 - entity\n"
                                            "result_type");
  ontology.constants = cp::SetOfEntities::fromPddl("a b - entity\n"
                                                   "r1 r2 - result_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("fact_a\n"
                                                     "fact_b(?e - entity) - result_type\n"
                                                     "fact_c\n"
                                                     "fact_d\n"
                                                     "fact_e(?e - entity) - result_type\n"
                                                     "fact_f(?e - entity)",
                                                     ontology.types);

  std::map<std::string, cp::Action> actions;

  {
    std::vector<cp::Parameter> parameters1(1, cp::Parameter::fromStr("?e - e1", ontology.types));
    cp::Action actionObj1(cp::strToCondition("fact_d", ontology, {}, parameters1),
                          cp::strToWsModification("fact_d & not(fact_a) & assign(fact_b(?e), r1)", ontology, {}, parameters1));
    actionObj1.parameters = std::move(parameters1);
    actions.emplace(action1, actionObj1);
  }

  {
    cp::Action actionObj2({},
                          cp::strToWsModification("fact_d", ontology, {}, {}));
    actions.emplace(action2, actionObj2);
  }

  {
    cp::Action actionObj3({},
                          cp::strToWsModification("fact_c", ontology, {}, {}));
    actions.emplace(action3, actionObj3);
  }


  SetOfEvents setOfEvents;
  std::vector<Parameter> eventParameters{Parameter::fromStr("?e - entity", ontology.types)};
  cp::Event event(cp::strToCondition("fact_a", ontology, {}, eventParameters),
                  cp::strToWsModification("forall(?e - entity, when(fact_f(?e), set(fact_b(?e), fact_e(?e))))", ontology, {}, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);
  Domain domain(std::move(actions), ontology, std::move(setOfEvents));


  const auto& domainOntology = domain.getOntology();

  Problem problem;
  auto& entities = problem.entities;
  entities = cp::SetOfEntities::fromPddl("ent - entity\n"
                                         "sub_ent1 - e1\n"
                                         "sub_ent2 - e2", ontology.types);

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("fact_d", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: fact_d\n"
              "---------------------------\n"
              "actions: action2", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("=(fact_b(ent), r2)", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: fact_b(ent)=r2\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("!fact_c", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("not(=(fact_b(ent), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(ent)=r1\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("not(=(fact_b(sub_ent1), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent1)=r1\n"
              "---------------------------\n"
              "actions: action1\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("not(=(fact_b(sub_ent2), r1))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent2)=r1\n"
              "---------------------------\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
  }

  {
    _setGoalsForAPriority(problem, {cp::Goal::fromStr("not(=(fact_b(sub_ent1), r2))", domainOntology, entities)});
    problem.goalStack.refreshIfNeeded(domain);
    EXPECT_EQ("goal: !fact_b(sub_ent1)=r2\n"
              "---------------------------\n"
              "actions: action1\n"
              "events: soe_from_constructor|event", problem.goalStack.printGoalsCache());
  }
}

