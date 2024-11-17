#include <gtest/gtest.h>
#include <prioritizedgoalsplanner/prioritizedgoalsplanner.hpp>
#include <prioritizedgoalsplanner/types/predicate.hpp>
#include <prioritizedgoalsplanner/types/setofconstfacts.hpp>
#include <prioritizedgoalsplanner/types/setofevents.hpp>
#include <prioritizedgoalsplanner/util/serializer/deserializefrompddl.hpp>
#include <prioritizedgoalsplanner/util/trackers/goalsremovedtracker.hpp>
#include <prioritizedgoalsplanner/util/print.hpp>
#include "docexamples/test_planningDummyExample.hpp"
#include "docexamples/test_planningExampleWithAPreconditionSolve.hpp"


namespace
{
const auto _now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
const std::map<pgp::SetOfEventsId, pgp::SetOfEvents> _emptySetOfEvents;
const std::string _fact_a = "fact_a";
const std::string _fact_b = "fact_b";
const std::string _fact_c = "fact_c";


void _setGoalsForAPriority(pgp::Problem& pProblem,
                           const std::vector<pgp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = pgp::GoalStack::defaultPriority)
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow, pPriority);
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

pgp::ActionInvocationWithGoal _lookForAnActionToDoThenNotify(
    pgp::Problem& pProblem,
    const pgp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  auto plan = pgp::planForMoreImportantGoalPossible(pProblem, pDomain, true, pNow);
  if (!plan.empty())
  {
    auto& firstActionInPlan = plan.front();
    notifyActionStarted(pProblem, pDomain, firstActionInPlan, pNow);
    notifyActionDone(pProblem, pDomain, firstActionInPlan, pNow);
    return firstActionInPlan;
  }
  return pgp::ActionInvocationWithGoal("", std::map<pgp::Parameter, pgp::Entity>(), {}, 0);
}



void _simplest_plan_possible()
{
  const std::string action1 = "action1";
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("type1 type2 - entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - type1\n"
                                                    "titi - type2", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a(?e - entity)\n"
                                                      "pred_b\n", ontology.types);

  const pgp::SetOfEntities entities;

  std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?pa - type1", ontology.types));
  pgp::Action actionObj1(pgp::strToCondition("pred_a(?pa)", ontology, entities, parameters),
                         pgp::strToWsModification("pred_b", ontology, entities, parameters));
  actionObj1.parameters = std::move(parameters);
  actions.emplace(action1, actionObj1);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_b", ontology, entities)});
  problem.worldState.addFact(pgp::Fact("pred_a(toto)", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);

  EXPECT_EQ("action1(?pa -> toto)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}



void _wrong_condition_type()
{
  const std::string action1 = "action1";
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity\n"
                                             "type1 - entity\n"
                                             "type2 - entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - type1\n"
                                                    "titi - type2", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a(?e - entity)\n"
                                                      "pred_b\n", ontology.types);

  const pgp::SetOfEntities entities;

  std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?pa - type1", ontology.types));
  pgp::Action actionObj1(pgp::strToCondition("pred_a(?pa)", ontology, entities, parameters),
                         pgp::strToWsModification("pred_b", ontology, entities, parameters));
  actionObj1.parameters = std::move(parameters);
  actions.emplace(action1, actionObj1);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_b", ontology, entities)});
  problem.worldState.addFact(pgp::Fact("pred_a(titi)", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);

  EXPECT_EQ("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _number_type()
{
  const std::string action1 = "action1";
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto - entity", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a(?e - entity) - number\n"
                                                      "pred_b", ontology.types);

  const pgp::SetOfEntities entities;

  pgp::Action actionObj1(pgp::strToCondition("pred_a(toto)=10", ontology, entities, {}),
                         pgp::strToWsModification("pred_b", ontology, entities, {}));
  actions.emplace(action1, actionObj1);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_b", ontology, entities)});
  EXPECT_EQ("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
  problem.worldState.addFact(pgp::Fact("pred_a(toto)=10", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);

  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_b", ontology, entities)});
  EXPECT_EQ(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _planWithActionThenEventWithFluentParameter()
{
  const std::string action1 = "action1";
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto titi - entity", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a - entity\n"
                                                      "pred_b(?e - entity)", ontology.types);

  const pgp::SetOfEntities entities;

  pgp::Action actionObj1({},
                         pgp::strToWsModification("pred_a=toto", ontology, entities, {}));
  actions.emplace(action1, actionObj1);

  pgp::SetOfEvents setOfEvents;
  std::vector<pgp::Parameter> eventParameters{pgp::Parameter::fromStr("?e - entity", ontology.types)};
  pgp::Event event(pgp::strToCondition("pred_a=?e", ontology, entities, eventParameters),
                   pgp::strToWsModification("pred_b(?e)", ontology, entities, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);

  pgp::Domain domain(std::move(actions), {}, std::move(setOfEvents));
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_b(toto)", ontology, entities)});
  EXPECT_EQ(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _planWithActionThenEventWithAssign()
{
  const std::string action1 = "action1";
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity\n"
                                             "other_type");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto titi - entity\n"
                                                    "v - other_type", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a - other_type\n"
                                                      "pred_b(?e - entity) - other_type\n"
                                                      "pred_c - other_type\n"
                                                      "pred_d - other_type", ontology.types);

  const pgp::SetOfEntities entities;

  std::vector<pgp::Parameter> actionParameters{pgp::Parameter::fromStr("?e - entity", ontology.types)};
  pgp::Action actionObj1({},
                         pgp::strToWsModification("assign(pred_a, pred_b(?e))", ontology, entities, actionParameters));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  pgp::SetOfEvents setOfEvents;
  std::vector<pgp::Parameter> eventParameters{pgp::Parameter::fromStr("?t - other_type", ontology.types)};
  pgp::Event event(pgp::strToCondition("pred_a=?t", ontology, entities, eventParameters),
                   pgp::strToWsModification("pred_d=?t", ontology, entities, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);

  pgp::Domain domain(std::move(actions), {}, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_d=v", ontology, entities)});
  problem.worldState.addFact(pgp::Fact("pred_b(toto)=v", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  EXPECT_EQ(action1 + "(?e -> toto)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _fluentEqualityInPrecoditionOfAnAction()
{
  std::map<std::string, pgp::Action> actions;

  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("entity\n"
                                             "other_type\n"
                                             "lol");
  ontology.constants = pgp::SetOfEntities::fromPddl("toto titi - entity\n"
                                                    "v - other_type\n"
                                                    "lol_val - lol", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("pred_a - other_type\n"
                                                      "pred_b(?e - entity) - other_type\n"
                                                      "pred_c(?l - lol) - other_type\n"
                                                      "pred_d(?l - lol)", ontology.types);
  const pgp::SetOfEntities entities;

  const std::string action1 = "action1";
  std::vector<pgp::Parameter> actionParameters{pgp::Parameter::fromStr("?e - entity", ontology.types)};
  pgp::Action actionObj1({},
                         pgp::strToWsModification("assign(pred_a, pred_b(?e))", ontology, entities, actionParameters));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  const std::string action2 = "action2";
  std::vector<pgp::Parameter> action2Parameters{pgp::Parameter::fromStr("?l - lol", ontology.types)};
  pgp::Action actionObj2(pgp::strToCondition("=(pred_a, pred_c(?l))", ontology, entities, action2Parameters),
                         pgp::strToWsModification("pred_d(?l)", ontology, entities, action2Parameters));
  actionObj2.parameters = std::move(action2Parameters);
  actions.emplace(action2, actionObj2);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  pgp::Problem problem;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("pred_d(lol_val)", ontology, entities)});
  problem.worldState.addFact(pgp::Fact("pred_b(toto)=v", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("pred_c(lol_val)=v", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  EXPECT_EQ(action1 + "(?e -> toto)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _testIncrementOfVariables()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("");
  ontology.constants = pgp::SetOfEntities::fromPddl("", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("numberOfQuestion - number\n"
                                                      "maxNumberOfQuestions - number\n"
                                                      "ask_all_the_questions\n"
                                                      "finished_to_ask_questions", ontology.types);
  const pgp::SetOfEntities entities;

  const std::string action_askQuestion1 = "ask_question_1";
  const std::string action_askQuestion2 = "ask_question_2";
  const std::string action_finisehdToAskQuestions = "finish_to_ask_questions";
  const std::string action_sayQuestionBilan = "say_question_bilan";

  std::map<std::string, pgp::Action> actions;
  const pgp::Action actionQ1({}, pgp::strToWsModification("ask_all_the_questions & add(numberOfQuestion, 1)", ontology, entities, {}));
  const pgp::Action actionFinishToActActions(pgp::strToCondition("equals(numberOfQuestion, maxNumberOfQuestions)", ontology, entities, {}),
                                             pgp::strToWsModification("ask_all_the_questions", ontology, entities, {}));
  const pgp::Action actionSayQuestionBilan(pgp::strToCondition("ask_all_the_questions", ontology, entities, {}),
                                           pgp::strToWsModification("finished_to_ask_questions", ontology, entities, {}));
  actions.emplace(action_askQuestion1, actionQ1);
  actions.emplace(action_askQuestion2, pgp::Action({}, pgp::strToWsModification("ask_all_the_questions & add(numberOfQuestion, 1)", ontology, entities, {})));
  actions.emplace(action_finisehdToAskQuestions, actionFinishToActActions);
  actions.emplace(action_sayQuestionBilan, actionSayQuestionBilan);
  pgp::Domain domain(std::move(actions), ontology);

  std::string initFactsStr = "numberOfQuestion=0 & maxNumberOfQuestions=3";
  pgp::Problem problem;
  problem.worldState.modify(pgp::strToWsModification(initFactsStr, ontology, entities, {}), problem.goalStack, _emptySetOfEvents, ontology, entities, _now);
  assert(pgp::strToCondition(initFactsStr, ontology, entities, {})->isTrue(problem.worldState));
  assert(!actionFinishToActActions.precondition->isTrue(problem.worldState));
  assert(!actionSayQuestionBilan.precondition->isTrue(problem.worldState));
  assert(pgp::strToCondition("equals(maxNumberOfQuestions, numberOfQuestion + 3)", ontology, entities, {})->isTrue(problem.worldState));
  assert(!pgp::strToCondition("equals(maxNumberOfQuestions, numberOfQuestion + 4)", ontology, entities, {})->isTrue(problem.worldState));
  assert(pgp::strToCondition("equals(maxNumberOfQuestions, numberOfQuestion + 4 - 1)", ontology, entities, {})->isTrue(problem.worldState));
  for (std::size_t i = 0; i < 3; ++i)
  {
    _setGoalsForAPriority(problem, {pgp::Goal::fromStr("finished_to_ask_questions", ontology, entities)});
    auto actionToDo = _lookForAnActionToDo(problem, domain).actionInvocation.toStr();
    if (i == 0 || i == 2)
      EXPECT_EQ(action_askQuestion1, actionToDo);
    else
      EXPECT_EQ(action_askQuestion2, actionToDo);
    problem.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    problem.worldState.modify(itAction->second.effect.worldStateModification, problem.goalStack,
                              _emptySetOfEvents, ontology, entities, _now);
    problem.worldState.modify(pgp::strToWsModification("!ask_all_the_questions", ontology, entities, {}),
                              problem.goalStack, _emptySetOfEvents, ontology, entities, _now);
  }
  assert(actionFinishToActActions.precondition->isTrue(problem.worldState));
  assert(!actionSayQuestionBilan.precondition->isTrue(problem.worldState));
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("finished_to_ask_questions", ontology, entities)});
  auto actionToDo = _lookForAnActionToDo(problem, domain).actionInvocation.toStr();
  EXPECT_EQ(action_finisehdToAskQuestions, actionToDo);
  problem.historical.notifyActionDone(actionToDo);
  auto itAction = domain.actions().find(actionToDo);
  assert(itAction != domain.actions().end());
  problem.worldState.modify(itAction->second.effect.worldStateModification, problem.goalStack,
                            _emptySetOfEvents, ontology, entities, _now);
  EXPECT_EQ(action_sayQuestionBilan, _lookForAnActionToDo(problem, domain).actionInvocation.toStr());
  assert(actionFinishToActActions.precondition->isTrue(problem.worldState));
  assert(actionSayQuestionBilan.precondition->isTrue(problem.worldState));
  problem.worldState.modify(actionSayQuestionBilan.effect.worldStateModification, problem.goalStack,
                            _emptySetOfEvents, ontology, entities, _now);
}


void _actionWithParametersInPreconditionsAndEffects()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("");
  ontology.constants = pgp::SetOfEntities::fromPddl("", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("isEngaged(?hid - number)\n"
                                                      "isHappy(?hid - number)", ontology.types);
  const pgp::SetOfEntities entities;

  std::map<std::string, pgp::Action> actions;
  std::vector<pgp::Parameter> parameters(1, pgp::Parameter::fromStr("?human - number", ontology.types));
  pgp::Action joke(pgp::strToCondition("isEngaged(?human)", ontology, entities, parameters),
                   pgp::strToWsModification("isHappy(?human)", ontology, entities, parameters));
  joke.parameters = std::move(parameters);
  const std::string action1 = "action1";
  actions.emplace(action1, joke);

  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem;
  problem.worldState.addFact(pgp::Fact("isEngaged(1)", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);

  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("isHappy(1)", ontology, entities)});
  const auto& plan = pgp::planForEveryGoals(problem, domain, _now);
  EXPECT_EQ(action1 + "(?human -> 1)", pgp::planToStr(plan));
  EXPECT_EQ("00: (" + action1 + " 1) [1]\n", pgp::planToPddl(plan, domain));
}


void _testQuiz()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("");
  ontology.constants = pgp::SetOfEntities::fromPddl("", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("numberOfQuestion - number\n"
                                                      "maxNumberOfQuestions - number\n"
                                                      "ask_all_the_questions\n"
                                                      "finished_to_ask_questions", ontology.types);
  const pgp::SetOfEntities entities;

  const std::string action_askQuestion1 = "ask_question_1";
  const std::string action_askQuestion2 = "ask_question_2";
  const std::string action_sayQuestionBilan = "say_question_bilan";

  std::map<std::string, pgp::Action> actions;
  pgp::ProblemModification questionEffect(pgp::strToWsModification("add(numberOfQuestion, 1)", ontology, entities, {}));
  questionEffect.potentialWorldStateModification = pgp::strToWsModification("ask_all_the_questions", ontology, entities, {});
  const pgp::Action actionQ1({}, questionEffect);
  const pgp::Action actionSayQuestionBilan(pgp::strToCondition("ask_all_the_questions", ontology, entities, {}),
                                           pgp::strToWsModification("finished_to_ask_questions", ontology, entities, {}));
  actions.emplace(action_askQuestion1, actionQ1);
  actions.emplace(action_askQuestion2, pgp::Action({}, questionEffect));
  actions.emplace(action_sayQuestionBilan, actionSayQuestionBilan);

  pgp::Domain domain(std::move(actions), {},
                     pgp::Event(pgp::strToCondition("equals(numberOfQuestion, maxNumberOfQuestions)", ontology, entities, {}),
                                pgp::strToWsModification("ask_all_the_questions", ontology, entities, {})));

  auto initFacts = pgp::strToWsModification("numberOfQuestion=0 & maxNumberOfQuestions=3", ontology, entities, {});

  pgp::Problem problem;

  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("finished_to_ask_questions", ontology, entities)});
  auto& setOfEventsMap = domain.getSetOfEvents();
  problem.worldState.modify(initFacts, problem.goalStack, setOfEventsMap, {}, {}, _now);
  for (std::size_t i = 0; i < 3; ++i)
  {
    auto actionToDo = _lookForAnActionToDo(problem, domain).actionInvocation.toStr();
    if (i == 0 || i == 2)
      EXPECT_EQ(action_askQuestion1, actionToDo);
    else
      EXPECT_EQ(action_askQuestion2, actionToDo);
    problem.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    problem.worldState.modify(itAction->second.effect.worldStateModification,
                              problem.goalStack, setOfEventsMap, {}, {}, _now);
  }

  auto actionToDo = _lookForAnActionToDo(problem, domain).actionInvocation.toStr();
  EXPECT_EQ(action_sayQuestionBilan, actionToDo);
}



void _doNextActionThatBringsToTheSmallerCost()
{
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("location\n"
                                             "object\n"
                                             "robot");
  ontology.constants = pgp::SetOfEntities::fromPddl("me - robot\n"
                                                    "obj1 obj2 - object\n"
                                                    "livingRoom kitchen bedroom - location", ontology.types);

  ontology.predicates = pgp::SetOfPredicates::fromStr("objectGrabable(?o - object)\n"
                                                      "locationOfRobot(?r - robot) - location\n"
                                                      "locationOfObject(?o - object) - location\n"
                                                      "grab(?r - robot) - object", ontology.types);
  const std::string action_navigate = "navigate";
  const std::string action_grab = "grab";
  const std::string action_ungrab = "ungrab";

  std::map<std::string, pgp::Action> actions;
  std::vector<pgp::Parameter> navParameters{pgp::Parameter::fromStr("?targetPlace - location", ontology.types)};
  pgp::Action navAction({}, pgp::strToWsModification("locationOfRobot(me)=?targetPlace", ontology, {}, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(action_navigate, navAction);

  std::vector<pgp::Parameter> grabParameters{pgp::Parameter::fromStr("?object - object", ontology.types)};
  pgp::Action grabAction(pgp::strToCondition("equals(locationOfRobot(me), locationOfObject(?object)) & !grab(me)=*", ontology, {}, grabParameters),
                         pgp::strToWsModification("grab(me)=?object", ontology, {}, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(action_grab, grabAction);

  std::vector<pgp::Parameter> ungrabParameters{pgp::Parameter::fromStr("?object - object", ontology.types)};
  pgp::Action ungrabAction({}, pgp::strToWsModification("!grab(me)=?object", ontology, {}, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(action_ungrab, ungrabAction);

  pgp::SetOfEvents setOfEvents;
  {
    std::vector<pgp::Parameter> eventParameters{pgp::Parameter::fromStr("?object - object", ontology.types), pgp::Parameter::fromStr("?location - location", ontology.types)};
    pgp::Event event(pgp::strToCondition("locationOfRobot(me)=?location & grab(me)=?object & objectGrabable(?object)", ontology, {}, eventParameters),
                     pgp::strToWsModification("locationOfObject(?object)=?location", ontology, {}, eventParameters));
    event.parameters = std::move(eventParameters);
    setOfEvents.add(event);
  }

  pgp::Domain domain(std::move(actions), {}, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem;
  auto& entities = problem.entities;
  problem.worldState.addFact(pgp::Fact("objectGrabable(obj1)", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("objectGrabable(obj2)", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("locationOfRobot(me)=livingRoom", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("grab(me)=obj2", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("locationOfObject(obj2)=livingRoom", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("locationOfObject(obj1)=kitchen", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  auto secondProblem = problem;
  auto thirdProblem = problem;
  auto fourthProblem = problem;
  // Here it will will be quicker for the second goal if we ungrab the obj2 right away
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("locationOfObject(obj1)=bedroom & !grab(me)=obj1", ontology, entities),
                                  pgp::Goal::fromStr("locationOfObject(obj2)=livingRoom & !grab(me)=obj2", ontology, entities)});
  const auto& plan = pgp::planForEveryGoals(problem, domain, _now);
  const std::string planStartingWithUngrab = "ungrab(?object -> obj2)\n"
                                             "navigate(?targetPlace -> kitchen)\n"
                                             "grab(?object -> obj1)\n"
                                             "navigate(?targetPlace -> bedroom)\n"
                                             "ungrab(?object -> obj1)";
  EXPECT_EQ(planStartingWithUngrab, pgp::planToStr(plan, "\n"));

  // Here it will will be quicker for the second goal if we move the obj2 to the kitchen
  _setGoalsForAPriority(secondProblem, {pgp::Goal::fromStr("locationOfObject(obj1)=bedroom & !grab(me)=obj1", ontology, entities),
                                        pgp::Goal::fromStr("locationOfObject(obj2)=kitchen & !grab(me)=obj2", ontology, entities)});
  const auto& secondPlan = pgp::planForEveryGoals(secondProblem, domain, _now);
  const std::string planStartingWithNavigate = "navigate(?targetPlace -> kitchen)\n"
                                               "ungrab(?object -> obj2)\n"
                                               "grab(?object -> obj1)\n"
                                               "navigate(?targetPlace -> bedroom)\n"
                                               "ungrab(?object -> obj1)";
  EXPECT_EQ(planStartingWithNavigate, pgp::planToStr(secondPlan, "\n"));

  // Exactly the same checks but !grab(me) part of goal before
  // ---------------------------------------------------------
  // Here it will will be quicker for the second goal if we ungrab the obj2 right away
  _setGoalsForAPriority(thirdProblem, {pgp::Goal::fromStr("!grab(me)=obj1 & locationOfObject(obj1)=bedroom", ontology, entities),
                                       pgp::Goal::fromStr("!grab(me)=obj2 & locationOfObject(obj2)=livingRoom", ontology, entities)});
  EXPECT_EQ(planStartingWithUngrab, pgp::planToStr(pgp::planForEveryGoals(thirdProblem, domain, _now), "\n"));

  // Here it will will be quicker for the second goal if we move the obj2 to the kitchen
  _setGoalsForAPriority(fourthProblem, {pgp::Goal::fromStr("!grab(me)=obj1 & locationOfObject(obj1)=bedroom", ontology, entities),
                                        pgp::Goal::fromStr("!grab(me)=obj2 & locationOfObject(obj2)=kitchen", ontology, entities)});
  EXPECT_EQ(planStartingWithNavigate, pgp::planToStr(pgp::planForEveryGoals(fourthProblem, domain, _now), "\n"));
}


void _satisfyGoalWithSuperiorOperator()
{
  const std::string action1 = "action1";
  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr("fact_a - number\n"
                                                      "fact_b", ontology.types);

  pgp::SetOfConstFacts timelessFacts;
  timelessFacts.add(pgp::Fact("fact_b", false, ontology, {}, {}));

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action(pgp::strToCondition("fact_b", ontology, {}, {}),
                                       pgp::strToWsModification("fact_a=100", ontology, {}, {})));
  pgp::Domain domain(std::move(actions), ontology, {}, {}, timelessFacts);
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem(&timelessFacts.setOfFacts());
  auto& entities = problem.entities;
  problem.worldState.addFact(pgp::Fact("fact_a=10", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("fact_a>50", ontology, entities)});

  EXPECT_EQ(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  EXPECT_EQ("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _parameterToFillFromConditionOfFirstAction()
{
  const std::string action1 = "action1";
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("location\n"
                                             "chargingZone");
  ontology.constants = pgp::SetOfEntities::fromPddl("cz - chargingZone\n"
                                                    "czLocation - location", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("locationOfRobot - location\n"
                                                      "declaredLocationOfChargingZone(?cz - chargingZone) - location\n"
                                                      "batteryLevel - number", ontology.types);

  std::map<std::string, pgp::Action> actions;
  std::vector<pgp::Parameter> actionParameters{pgp::Parameter::fromStr("?cz - chargingZone", ontology.types)};
  pgp::Action action1Obj(pgp::strToCondition("=(locationOfRobot, declaredLocationOfChargingZone(?cz))", ontology, {}, actionParameters),
                         pgp::strToWsModification("batteryLevel=100", ontology, {}, actionParameters));
  action1Obj.parameters = std::move(actionParameters);
  actions.emplace(action1, action1Obj);
  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem;
  auto& entities = problem.entities;
  problem.worldState.addFact(pgp::Fact("locationOfRobot=czLocation", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("declaredLocationOfChargingZone(cz)=czLocation", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  problem.worldState.addFact(pgp::Fact("batteryLevel=40", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("batteryLevel=100", ontology, entities)});

  EXPECT_EQ(action1 + "(?cz -> cz)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _planToMove()
{
  const std::string action1 = "action1";
  pgp::Ontology ontology;
  ontology.types = pgp::SetOfTypes::fromPddl("location\n"
                                             "graspable_obj - object");
  ontology.constants = pgp::SetOfEntities::fromPddl("loc1 - location\n"
                                                    "bottle - graspable_obj", ontology.types);
  ontology.predicates = pgp::SetOfPredicates::fromStr("locationOfRobot - location\n"
                                                      "locationOf(?o - object) - location", ontology.types);

  std::map<std::string, pgp::Action> actions;
  std::vector<pgp::Parameter> actionParameters{pgp::Parameter::fromStr("?o - object", ontology.types)};
  pgp::Action action1Obj({}, pgp::strToWsModification("assign(locationOfRobot, locationOf(?o))", ontology, {}, actionParameters));
  action1Obj.parameters = std::move(actionParameters);
  actions.emplace(action1, action1Obj);
  pgp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  pgp::Problem problem;
  auto& entities = problem.entities;
  problem.worldState.addFact(pgp::Fact("locationOf(bottle)=loc1", false, ontology, entities, {}), problem.goalStack, setOfEventsMap,
                             ontology, entities, _now);
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("locationOfRobot=loc1", ontology, entities)});

  EXPECT_EQ(action1 + "(?o -> bottle)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}

void _disjunctiveGoal()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr("fact_a\n"
                                                      "fact_b", ontology.types);

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action({}, pgp::strToWsModification("fact_a", ontology, {}, {})));
  actions.emplace(action2, pgp::Action({}, pgp::strToWsModification("fact_b", ontology, {}, {})));
  pgp::Domain domain(std::move(actions), ontology);

  pgp::Problem problem;
  auto& entities = problem.entities;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("or(fact_a, fact_b)", ontology, entities)});

  auto firstActionStr = _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr();
  if (firstActionStr != action1 && firstActionStr != action2)
    EXPECT_TRUE(false);
  EXPECT_EQ("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _disjunctivePrecondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  pgp::Ontology ontology;
  ontology.predicates = pgp::SetOfPredicates::fromStr("fact_a\n"
                                                      "fact_b\n"
                                                      "fact_c", ontology.types);

  std::map<std::string, pgp::Action> actions;
  actions.emplace(action1, pgp::Action({}, pgp::strToWsModification("fact_a", ontology, {}, {})));
  actions.emplace(action2, pgp::Action({}, pgp::strToWsModification("fact_b", ontology, {}, {})));
  actions.emplace(action3, pgp::Action(pgp::strToCondition("or(fact_a, fact_b)", ontology, {}, {}),
                                       pgp::strToWsModification("fact_c", ontology, {}, {})));
  pgp::Domain domain(std::move(actions), ontology);

  pgp::Problem problem;
  auto& entities = problem.entities;
  _setGoalsForAPriority(problem, {pgp::Goal::fromStr("fact_c", ontology, entities)});

  auto firstActionStr = _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr();
  if (firstActionStr != action1 && firstActionStr != action2)
    EXPECT_TRUE(false);
  EXPECT_EQ(action3, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  EXPECT_EQ("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


}



TEST(Planner, test_planner)
{
  planningDummyExample();
  planningExampleWithAPreconditionSolve();

  _simplest_plan_possible();
  _wrong_condition_type();
  _number_type();
  _planWithActionThenEventWithFluentParameter();
  _planWithActionThenEventWithAssign();
  _fluentEqualityInPrecoditionOfAnAction();
  _testIncrementOfVariables();
  _actionWithParametersInPreconditionsAndEffects();
  _testQuiz();
  _doNextActionThatBringsToTheSmallerCost();
  _satisfyGoalWithSuperiorOperator();
  _parameterToFillFromConditionOfFirstAction();
  _planToMove();
  _disjunctiveGoal();
  _disjunctivePrecondition();
}
