#include "test_plannerWithSingleType.hpp"
#include <iostream>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>
#include <contextualplanner/types/axiom.hpp>
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setofevents.hpp>
#include <contextualplanner/util/trackers/goalsremovedtracker.hpp>
#include <contextualplanner/util/print.hpp>

namespace
{
const std::map<cp::SetOfEventsId, cp::SetOfEvents> _emptySetOfEvents;
const std::string _sep = ", ";
const std::unique_ptr<std::chrono::steady_clock::time_point> _now = {};

const std::string _fact_a = "fact_a";
const std::string _fact_b = "fact_b";
const std::string _fact_c = "fact_c";
const std::string _fact_d = "fact_d";
const std::string _fact_e = "fact_e";
const std::string _fact_f = "fact_f";
const std::string _fact_g = "fact_g";
const std::string _fact_punctual_p1 = cp::Fact::punctualPrefix + "fact_p1";
const std::string _fact_punctual_p2 = cp::Fact::punctualPrefix + "fact_p2";
const std::string _fact_punctual_p3 = cp::Fact::punctualPrefix + "fact_p3";
const std::string _fact_punctual_p4 = cp::Fact::punctualPrefix + "fact_p4";
const std::string _fact_punctual_p5 = cp::Fact::punctualPrefix + "fact_p5";
const std::string _fact_advertised = "advertised";
const std::string _fact_beginOfConversation = "begin_of_conversation";
const std::string _fact_presented = "presented";
const std::string _fact_greeted = "greeted";
const std::string _fact_is_close = "is_close";
const std::string _fact_hasQrCode = "has_qrcode";
const std::string _fact_hasCheckInPasword = "has_check_in_password";
const std::string _fact_userWantsToCheckedIn = "user_wants_to_checked_in";
const std::string _fact_checkedIn = "checked_in";
const std::string _fact_beSad = "be_sad";
const std::string _fact_beHappy = "be_happy";
const std::string _fact_engagedWithUser = "engaged_with_user";
const std::string _fact_userSatisfied = "user_satisfied";
const std::string _fact_robotLearntABehavior = "robot_learnt_a_behavior";
const std::string _fact_headTouched = "head_touched";
const std::string _fact_punctual_headTouched = cp::Fact::punctualPrefix + "head_touched";
const std::string _fact_punctual_checkedIn = cp::Fact::punctualPrefix + "checked_in";

const std::string _action_presentation = "presentation";
const std::string _action_askQuestion1 = "ask_question_1";
const std::string _action_askQuestion2 = "ask_question_2";
const std::string _action_greet = "greet";
const std::string _action_advertise = "advertise";
const std::string _action_checkIn = "check_in";
const std::string _action_joke = "joke";
const std::string _action_checkInWithRealPerson = "check_in_with_real_person";
const std::string _action_checkInWithQrCode = "check_in_with_qrcode";
const std::string _action_checkInWithPassword = "check_in_with_password";
const std::string _action_goodBoy = "good_boy";
const std::string _action_navigate = "navigate";
const std::string _action_welcome = "welcome";
const std::string _action_grab = "grab";
const std::string _action_ungrab = "ungrab";
static const std::vector<cp::Parameter> _emptyParameters;

template <typename TYPE>
void assert_eq(const TYPE& pExpected,
               const TYPE& pValue)
{
  if (pExpected != pValue)
    assert(false);
}

template <typename TYPE>
void assert_true(const TYPE& pValue)
{
  if (!pValue)
    assert(false);
}

template <typename TYPE>
void assert_false(const TYPE& pValue)
{
  if (pValue)
    assert(false);
}


cp::Fact _fact(const std::string& pStr,
               const cp::Ontology& pOntology,
               const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::Fact(pStr, pOntology, {}, pParameters);
}

cp::Parameter _parameter(const std::string& pStr,
                         const cp::Ontology& pOntology) {
  return cp::Parameter::fromStr(pStr, pOntology.types);
}

cp::Entity _entityDeclaration(const std::string& pStr,
                              const cp::Ontology& pOntology) {
  return cp::Entity::fromDeclaration(pStr, pOntology.types);
}

cp::Goal _goal(const std::string& pStr,
               const cp::Ontology& pOntology,
               int pMaxTimeToKeepInactive = -1,
               const std::string& pGoalGroupId = "") {
  return cp::Goal(pStr, pOntology, {}, pMaxTimeToKeepInactive, pGoalGroupId);
}

std::unique_ptr<cp::Condition> _condition_fromStr(const std::string& pConditionStr,
                                                  const cp::Ontology& pOntology,
                                                  const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::Condition::fromStr(pConditionStr, pOntology, {}, pParameters);
}

std::unique_ptr<cp::WorldStateModification> _worldStateModification_fromStr(const std::string& pStr,
                                                                            const cp::Ontology& pOntology,
                                                                            const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::WorldStateModification::fromStr(pStr, pOntology, {}, pParameters);
}

void _setGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<std::string>& pGoalStrs,
                           const cp::Ontology& pOntology,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  std::vector<cp::Goal> goals;
  for (auto& currFactStr : pGoalStrs)
    goals.emplace_back(currFactStr, pOntology, cp::SetOfEntities());
  pProblem.goalStack.setGoals(goals, pProblem.worldState, pNow, pPriority);
}


void _setGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<cp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow, pPriority);
}


void _setGoals(cp::Problem& pProblem,
               const std::map<int, std::vector<std::string>>& pGoalStrs,
               const cp::Ontology& pOntology,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  std::map<int, std::vector<cp::Goal>> goals;
  for (auto& currGoals : pGoalStrs)
  {
    auto& goalsForAPrio = goals[currGoals.first];
    for (auto& currGoal : currGoals.second)
      goalsForAPrio.emplace_back(currGoal, pOntology, cp::SetOfEntities());
  }
  pProblem.goalStack.setGoals(goals, pProblem.worldState, pNow);
}


void _setGoals(cp::Problem& pProblem,
               const std::map<int, std::vector<cp::Goal>>& pGoals,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  pProblem.goalStack.setGoals(pGoals, pProblem.worldState, pNow);
}


void _addGoals(cp::Problem& pProblem,
               const std::map<int, std::vector<std::string>>& pGoalStrs,
               const cp::Ontology& pOntology,
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  std::map<int, std::vector<cp::Goal>> goals;
  for (auto& currGoals : pGoalStrs)
  {
    auto& goalsForAPrio = goals[currGoals.first];
    for (auto& currGoal : currGoals.second)
      goalsForAPrio.emplace_back(currGoal, pOntology, cp::SetOfEntities());
  }
  pProblem.goalStack.addGoals(goals, pProblem.worldState, pNow);
}

void _addGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<std::string>& pGoalStrs,
                           const cp::Ontology& pOntology,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  std::vector<cp::Goal> goals;
  for (auto& currFactStr : pGoalStrs)
    goals.emplace_back(currFactStr, pOntology, cp::SetOfEntities());
  pProblem.goalStack.addGoals(goals, pProblem.worldState, pNow, pPriority);
}


void _addGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<cp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  pProblem.goalStack.addGoals(pGoals, pProblem.worldState, pNow, pPriority);
}


void _setFacts(cp::WorldState& pWorldState,
               const std::set<std::string>& pFactStrs,
               cp::GoalStack& pGoalStack,
               const cp::Ontology& pOntology,
               const std::map<cp::SetOfEventsId, cp::SetOfEvents>& pSetOfEvents = _emptySetOfEvents) {
  std::set<cp::Fact> facts;
  for (auto& currFactStr : pFactStrs)
    facts.emplace(currFactStr, pOntology, cp::SetOfEntities(), std::vector<cp::Parameter>());
  pWorldState.setFacts(facts, pGoalStack, pSetOfEvents, pOntology, cp::SetOfEntities(), _now);
}

void _addFact(cp::WorldState& pWorldState,
              const std::string& pFactStr,
              cp::GoalStack& pGoalStack,
              const cp::Ontology& pOntology,
              const std::map<cp::SetOfEventsId, cp::SetOfEvents>& pSetOfEvents = _emptySetOfEvents,
              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {}) {
  pWorldState.addFact(_fact(pFactStr, pOntology), pGoalStack, pSetOfEvents, pOntology, cp::SetOfEntities(), pNow);
}

void _removeFact(cp::WorldState& pWorldState,
                 const std::string& pFactStr,
                 cp::GoalStack& pGoalStack,
                 const cp::Ontology& pOntology,
                 const std::map<cp::SetOfEventsId, cp::SetOfEvents>& pSetOfEvents = _emptySetOfEvents,
                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {}) {
  pWorldState.removeFact(_fact(pFactStr, pOntology), pGoalStack, pSetOfEvents, pOntology, cp::SetOfEntities(), pNow);
}

bool _hasFact(cp::WorldState& pWorldState,
              const std::string& pFactStr,
              const cp::Ontology& pOntology) {
  return pWorldState.hasFact(_fact(pFactStr, pOntology));
}


std::string _solveStrConst(const cp::Problem& pProblem,
                           const std::map<std::string, cp::Action>& pActions,
                           const cp::Ontology& pOntology,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  cp::Domain domain(pActions, pOntology);
  return cp::planToStr(cp::planForEveryGoals(problem, domain, {}, pGlobalHistorical), _sep);
}

std::string _solveStrConst(const cp::Problem& pProblem,
                           const cp::Domain& pDomain,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  return cp::planToStr(cp::planForEveryGoals(problem, pDomain, {}, pGlobalHistorical), _sep);
}

std::string _solveStr(cp::Problem& pProblem,
                      const std::map<std::string, cp::Action>& pActions,
                      const cp::Ontology& pOntology,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions, pOntology);
  return cp::planToStr(cp::planForEveryGoals(pProblem, domain, pNow, pGlobalHistorical), _sep);
}

std::string _solveStr(cp::Problem& pProblem,
                      const cp::Domain& pDomain,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                      cp::Historical* pGlobalHistorical = nullptr)
{
  return cp::planToStr(cp::planForEveryGoals(pProblem, pDomain, pNow, pGlobalHistorical), _sep);
}


std::string _getGoalsDoneDuringAPlannificationConst(const cp::Problem& pProblem,
                                                    const cp::Domain& pDomain,
                                                    cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  std::list<cp::Goal> pGoalsDone;
  cp::planForEveryGoals(problem, pDomain, {}, pGlobalHistorical, &pGoalsDone);
  return cp::goalsToStr(pGoalsDone, _sep);
}

cp::ActionInvocationWithGoal _lookForAnActionToDo(cp::Problem& pProblem,
                                                  const cp::Domain& pDomain,
                                                  const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                                  const cp::Historical* pGlobalHistorical = nullptr)
{
  auto plan = cp::planForMoreImportantGoalPossible(pProblem, pDomain, true, pNow, pGlobalHistorical);
  if (!plan.empty())
    return plan.front();
  return cp::ActionInvocationWithGoal("", {}, {}, 0);
}

cp::ActionInvocationWithGoal _lookForAnActionToDoConst(const cp::Problem& pProblem,
                                                       const cp::Domain& pDomain,
                                                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                                       const cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  auto plan = cp::planForMoreImportantGoalPossible(problem, pDomain, true, pNow, pGlobalHistorical);
  if (!plan.empty())
    return plan.front();
  return cp::ActionInvocationWithGoal("", {}, {}, 0);
}

std::string _lookForAnActionToDoStr(cp::Problem& pProblem,
                                    const cp::Domain& pDomain,
                                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                    const cp::Historical* pGlobalHistorical = nullptr)
{
  return _lookForAnActionToDo(pProblem, pDomain, pNow, pGlobalHistorical).actionInvocation.toStr();
}

std::string _lookForAnActionToDoConstStr(const cp::Problem& pProblem,
                                         const cp::Domain& pDomain,
                                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                         const cp::Historical* pGlobalHistorical = nullptr)
{
  return _lookForAnActionToDoConst(pProblem, pDomain, pNow, pGlobalHistorical).actionInvocation.toStr();
}

cp::ActionInvocationWithGoal _lookForAnActionToDoThenNotify(
    cp::Problem& pProblem,
    const cp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  auto plan = cp::planForMoreImportantGoalPossible(pProblem, pDomain, true, pNow);
  if (!plan.empty())
  {
    auto& firstActionInPlan = plan.front();
    notifyActionStarted(pProblem, pDomain, firstActionInPlan, pNow);
    notifyActionDone(pProblem, pDomain, firstActionInPlan, pNow);
    return firstActionInPlan;
  }
  return cp::ActionInvocationWithGoal("", {}, {}, 0);
}


std::string _lookForAnActionToDoInParallelThenNotifyToStr(
    cp::Problem& pProblem,
    const cp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  auto actions = cp::actionsToDoInParallelNow(pProblem, pDomain, pNow);
  for (auto& currAction : actions)
  {
    notifyActionStarted(pProblem, pDomain, currAction, pNow);
    notifyActionDone(pProblem, pDomain, currAction, pNow);
  }
  return cp::planToStr(actions);
}




void _test_createEmptyGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("goal_name", ontology.types);
  _goal("goal_name", ontology, -1, "");
}

void _test_goalToStr()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("a\n"
                                                     "b\n"
                                                     "condition\n"
                                                     "goal_name", ontology.types);

  assert_eq<std::string>("persist(a & b)", _goal("persist(a & b)", ontology).toStr());
  assert_eq<std::string>("imply(condition, goal_name)", _goal("imply(condition, goal_name)", ontology).toStr());
  assert_eq<std::string>("persist(imply(condition, goal_name))", _goal("persist(imply(condition, goal_name))", ontology).toStr());
  assert_eq<std::string>("oneStepTowards(goal_name)", _goal("oneStepTowards(goal_name)", ontology).toStr());
}

void _test_factToStr()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("human");
  ontology.constants = cp::SetOfEntities::fromStr("h1 - human", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("isEngaged(?h - human)", ontology.types);
  assert_eq<std::string>("isEngaged(h1)", _fact("isEngaged(h1)", ontology).toStr());
}




void _test_conditionParameters()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location\n"
                                           "entity\n"
                                           "robot - physical_object");
  ontology.constants = cp::SetOfEntities::fromStr("me self - robot\n"
                                                  "pen - physical_object\n"
                                                  "entrance kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?r - robot) - location\n"
                                                     "grab(?r - robot, ?e - entity)\n"
                                                     "sf\n"
                                                     "ent - entity\n"
                                                     "n1 - number\n"
                                                     "n2 - number\n"
                                                     "a\n"
                                                     "b\n"
                                                     "loc - number\n"
                                                     "at(?po - physical_object, ?l - location)\n"
                                                     "distance(?po - physical_object, ?l - location) - number", ontology.types);

  assert_false(_condition_fromStr("", ontology).operator bool());

  std::vector<cp::Parameter> parameters = {_parameter("?target - location", ontology), _parameter("?object - entity", ontology)};
  std::map<cp::Parameter, cp::Entity> parametersToEntities = {{_parameter("?target - location", ontology), _entityDeclaration("kitchen - location", ontology)}, {_parameter("?object - location", ontology), _entityDeclaration("chair - entity", ontology)}};
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)",
                         _condition_fromStr("location(me)=?target & grab(me, ?object)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)",
                         _condition_fromStr("and(location(me)=?target, grab(me, ?object))", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair) & sf",
                         _condition_fromStr("and(location(me)=?target, grab(me, ?object), sf)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | sf",
                         _condition_fromStr("location(me)=?target | grab(me, ?object) | sf", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | sf",
                         _condition_fromStr("or(location(me)=?target, grab(me, ?object), sf)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("equals(n1, n2 + 3)", _condition_fromStr("equals(n1, n2 + 3)", ontology)->toStr());
  assert_eq<std::string>("!sf", _condition_fromStr("!sf", ontology)->toStr());
  assert_eq<std::string>("!sf", _condition_fromStr("not(sf)", ontology)->toStr());
  assert_eq<std::string>("n1!=2", _condition_fromStr("n1!=2", ontology)->toStr());
  assert_eq<std::string>("n1>3", _condition_fromStr("n1>3", ontology)->toStr());
  assert_eq<std::string>("n1>3", _condition_fromStr(">(n1, 3)", ontology)->toStr());
  assert_eq<std::string>("n1<3", _condition_fromStr("<(n1, 3)", ontology)->toStr());
  assert_eq<std::string>("!a=*", _condition_fromStr("=(a, undefined)", ontology)->toStr());
  assert_eq<std::string>("!location(me)=*", _condition_fromStr("=(location(me), undefined)", ontology)->toStr());
  assert_eq<std::string>("!location(me)=*", _condition_fromStr("=(location(me), undefined)", ontology)->toStr());
  assert_eq<std::string>("equals(n1, n2)", _condition_fromStr("=(n1, n2())", ontology)->toStr());
  assert_eq<std::string>("location(me)=entrance", _condition_fromStr("=(location(me), entrance)", ontology)->toStr());
  assert_eq<std::string>("equals(location(me), loc)", _condition_fromStr("=(location(me), loc())", ontology)->toStr());
  assert_eq<std::string>("equals(n1, n2)", _condition_fromStr("equals(n1, n2())", ontology)->toStr());
  assert_eq<std::string>("equals(location(me), loc)", _condition_fromStr("equals(location(me), loc())", ontology)->toStr());
  assert_eq<std::string>("exists(?l - location, at(self, ?l))", _condition_fromStr("exists(?l - location, at(self, ?l))", ontology)->toStr());
  assert_eq<std::string>("exists(?l - location, at(self, ?l) & at(pen, ?l))", _condition_fromStr("exists(?l - location, at(self, ?l) & at(pen, ?l))", ontology)->toStr());
  assert_eq<std::string>("!exists(?l - location, at(self, ?l))", _condition_fromStr("not(exists(?l - location, at(self, ?l)))", ontology)->toStr());
  assert_eq<std::string>("!(equals(distance(pen, entrance), distance(self, kitchen)))", _condition_fromStr("not(equals(distance(pen, entrance), distance(self, kitchen)))", ontology)->toStr());
  assert_eq<std::string>("!(equals(distance(pen, entrance), distance(self, kitchen)))", _condition_fromStr("not(=(distance(pen, entrance), distance(self, kitchen)))", ontology)->toStr());
}

void _test_wsModificationToStr()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location\n"
                                           "entity\n"
                                           "robot - physical_object");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "sweets - entity\n"
                                                  "target - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?r - robot) - location\n"
                                                     "locationEntity(?e - entity) - location\n"
                                                     "checkPointLocation - location\n"
                                                     "grab(?e - entity)\n"
                                                     "is_near(?e - entity, ?r - robot)\n"
                                                     "a\n"
                                                     "n1 - number\n"
                                                     "n2 - number", ontology.types);

  assert_false(_worldStateModification_fromStr("", ontology).operator bool());
  assert_eq<std::string>("location(me)=target", _worldStateModification_fromStr("location(me)=target", ontology)->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("location(me)=target & grab(sweets)", ontology)->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("and(location(me)=target, grab(sweets))", ontology)->toStr());
  assert_eq<std::string>("assign(n1, n2 + 3)", _worldStateModification_fromStr("assign(n1, n2 + 3)", ontology)->toStr());
  assert_eq<std::string>("assign(n1, n2 + 4 - 1)", _worldStateModification_fromStr("set(n1, n2 + 4 - 1)", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("increase(n1, 1)", _worldStateModification_fromStr("add(n1, 1)", ontology)->toStr());
  assert_eq<std::string>("increase(n1, 1)", _worldStateModification_fromStr("increase(n1, 1)", ontology)->toStr());
  assert_eq<std::string>("decrease(n1, 2)", _worldStateModification_fromStr("decrease(n1, 2)", ontology)->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("!a", ontology)->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("not(a)", ontology)->toStr());
  assert_eq<std::string>("!n1=*", _worldStateModification_fromStr("assign(n1, undefined)", ontology)->toStr());
  assert_eq<std::string>("forall(?e - entity, grab(?e), is_near(?e, me))", _worldStateModification_fromStr("forall(?e - entity, grab(?e), is_near(?e, me))", ontology)->toStr());
  assert_eq<std::string>("forall(?e - entity, grab(?e), is_near(?e, me))", _worldStateModification_fromStr("forall(?e - entity, when(grab(?e), is_near(?e, me)))", ontology)->toStr());
  assert_eq<std::string>("forall(?e - entity, grab(?e), !is_near(?e, me))", _worldStateModification_fromStr("forall(?e - entity, when(grab(?e), not(is_near(?e, me))))", ontology)->toStr());
  assert_eq<std::string>("assign(location(me), locationEntity(sweets))", _worldStateModification_fromStr("assign(location(me), locationEntity(sweets))", ontology)->toStr());
  assert_eq<std::string>("assign(location(me), locationEntity(sweets))", _worldStateModification_fromStr("set(location(me), locationEntity(sweets))", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("assign(location(me), checkPointLocation)", _worldStateModification_fromStr("assign(location(me), checkPointLocation())", ontology)->toStr()); // c() means that c is a predicate
  assert_eq<std::string>("location(me)=target", _worldStateModification_fromStr("assign(location(me), target)", ontology)->toStr());
  assert_eq<std::string>("assign(location(me), checkPointLocation)", _worldStateModification_fromStr("set(location(me), checkPointLocation())", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("assign(location(me), checkPointLocation)", _worldStateModification_fromStr("set(location(me), checkPointLocation)", ontology)->toStr()); // set is depecated
}

void _test_invertCondition()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location\n"
                                           "entity\n"
                                           "robot - physical_object");
  ontology.constants = cp::SetOfEntities::fromStr("me self - robot\n"
                                                  "chair - entity\n"
                                                  "pen - physical_object\n"
                                                  "kitchen entrance - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?r - robot) - location\n"
                                                     "locationEntity(?e - entity) - location\n"
                                                     "grab(?r - robot, ?e - entity)\n"
                                                     "distance(?po - physical_object, ?l - location) - number\n"
                                                     "se", ontology.types);

  assert_eq<std::string>("!location(me)=kitchen | grab(me, chair) | !se", _condition_fromStr("location(me)=kitchen & !grab(me, chair) & se)", ontology)->clone(nullptr, true)->toStr());
  assert_eq<std::string>("!location(me)=kitchen & !grab(me, chair) & se", _condition_fromStr("location(me)=kitchen | grab(me, chair) | !se", ontology)->clone(nullptr, true)->toStr());
  assert_eq<std::string>("equals(distance(pen, entrance), distance(self, kitchen))", _condition_fromStr("not(=(distance(pen, entrance), distance(self, kitchen)))", ontology)->clone(nullptr, true)->toStr());
}


void _test_checkCondition()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("b c d - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("a - entity", ontology.types);

  cp::WorldState worldState;
  cp::GoalStack goalStack;
  std::map<cp::SetOfEventsId, cp::SetOfEvents> setOfEvents;
  worldState.addFact(_fact("a=c", ontology), goalStack, setOfEvents, {}, {}, {});
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_false(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  assert_eq<std::size_t>(1, worldState.facts().size());
  worldState.addFact(_fact("a!=c", ontology), goalStack, setOfEvents, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_false(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a!=b", ontology), goalStack, setOfEvents, {}, {}, {});
  assert_eq<std::size_t>(2, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a=d", ontology), goalStack, setOfEvents, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a=d", ontology)->isTrue(worldState));
  assert_false(_condition_fromStr("a!=d", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a!=c", ontology), goalStack, setOfEvents, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a=d", ontology)->isTrue(worldState));
}


void _automaticallyRemoveGoalsWithAMaxTimeToKeepInactiveEqualTo0()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_advertised + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_checkedIn, ontology.types);
  cp::GoalStack goalStack;
  cp::WorldState worldState;
  assert_eq<std::size_t>(0u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_advertised, ontology), worldState, {}, 10);
  goalStack.pushBackGoal(_goal(_fact_beHappy, ontology), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_checkedIn, ontology, 0), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
  assert_eq<std::size_t>(1u, goalStack.goals().find(9)->second.size());
}


void _maxTimeToKeepInactiveEqualTo0UnderAnAlreadySatisfiedGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_checkedIn, ontology.types);
  cp::GoalStack goalStack;
  cp::WorldState worldState;
  assert_eq<std::size_t>(0u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal("persist(!" + _fact_a + ")", ontology), worldState, {}, 10);
  assert_eq<std::size_t>(1u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_checkedIn, ontology, 0), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
}

void _noPreconditionGoalImmediatlyReached()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("be_happy", ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr("be_happy", ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"}, ontology);
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain));
  assert_true(!problem.goalStack.goals().empty());
  assert_true(!_hasFact(problem.worldState, "be_happy", ontology));
  _addFact(problem.worldState, "be_happy", problem.goalStack, ontology);
  assert_true(_hasFact(problem.worldState, "be_happy", ontology));
}


void _removeGoalWhenItIsSatisfiedByAnAction()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("be_happy", ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr("be_happy", ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"}, ontology);

  auto plannerResult = _lookForAnActionToDoThenNotify(problem, domain);
  assert_eq(_action_goodBoy, plannerResult.actionInvocation.toStr());
  assert_eq<std::string>("be_happy", plannerResult.fromGoal->toStr());
  assert_eq(10, plannerResult.fromGoalPriority);
  assert_true(problem.goalStack.goals().empty());
}


void _removeAnAction()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("be_happy", ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr("be_happy", ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"}, ontology);
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  domain.removeAction(_action_goodBoy);
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _removeSomeGoals()
{
  const std::string goalGroupId = "greetAndCheckIn";
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("be_happy\n"
                                                     "checked_in\n"
                                                     "greeted", ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr("greeted", ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr("checked_in", ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr("be_happy", ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"}, ontology);
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  problem.goalStack.pushFrontGoal(_goal("checked_in", ontology, -1, goalGroupId), problem.worldState, {});
  problem.goalStack.pushFrontGoal(_goal("greeted", ontology, -1, goalGroupId), problem.worldState, {});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq<std::size_t>(0, goalsRemoved.size());
  assert_true(problem.goalStack.removeGoals(goalGroupId, problem.worldState, {}));
  assert_eq<std::size_t>(2, goalsRemoved.size());
  assert_false(problem.goalStack.removeGoals(goalGroupId, problem.worldState, {}));
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  problem.goalStack.clearGoals(problem.worldState, {});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
  onGoalsRemovedConnection.disconnect();
}


void _notifyGoalRemovedWhenItIsImmediatlyRemoved()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);

  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });

  problem.goalStack.addGoals({{10, {_goal(_fact_a, ontology)}}}, problem.worldState, {});

  problem.goalStack.addGoals({{9, {_goal(_fact_b, ontology, 0)}}}, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushBackGoal(_goal(_fact_c, ontology, 0), problem.worldState, {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_c, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushFrontGoal(_goal(_fact_d, ontology, 0), problem.worldState, {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_d, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushFrontGoal(_goal(_fact_e, ontology, 0), problem.worldState, {}, 11);
  assert_eq<std::size_t>(0u, goalsRemoved.size());
  problem.goalStack.changeGoalPriority(_fact_e, 9, true, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_e, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.setGoals({{10, {_goal(_fact_a, ontology, 0)}}, {9, {_goal(_fact_b, ontology, 0)}}}, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
  onGoalsRemovedConnection.disconnect();
}



void _handlePreconditionWithNegatedFacts()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_userSatisfied, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(_condition_fromStr("!" + _fact_checkedIn, ontology),
                                            _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_joke, cp::Action(_condition_fromStr("!" + _fact_checkedIn, ontology),
                                           _worldStateModification_fromStr(_fact_userSatisfied, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_userSatisfied, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_checkedIn}, ontology);
  auto actionToDo = _lookForAnActionToDoStr(problem, domain);
  assert_true(actionToDo == _action_greet || actionToDo == _action_joke);
}


void _testWithNegatedAccessibleFacts()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_e + " & !" + _fact_b, ontology),
                                      _worldStateModification_fromStr("!" + _fact_c, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b, ontology)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_a + " & !" + _fact_c, ontology),
                                      _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_a, _fact_b, _fact_c}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_d}, ontology);
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
  _addFact(problem.worldState, _fact_e,  problem.goalStack, ontology);
  assert_eq(action2, _lookForAnActionToDoStr(problem, domain));
}

void _noPlanWithALengthOf2()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}


void _noPlanWithALengthOf3()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}

void _2preconditions()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}

void _2Goals()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}

void _2UnrelatedGoals()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}


void _impossibleGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy}, ontology);
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, domain));
}


void _privigelizeTheActionsThatHaveManyPreferedInContext()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_hasQrCode + "\n" +
                                                     _fact_hasCheckInPasword + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_checkInWithQrCode, cp::Action(_condition_fromStr(_fact_hasQrCode, ontology),
                                                        _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                        _condition_fromStr(_fact_hasQrCode, ontology)));
  actions.emplace(_action_checkInWithPassword, cp::Action(_condition_fromStr(_fact_hasCheckInPasword, ontology),
                                                          _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                          _condition_fromStr(_fact_hasCheckInPasword, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, domain));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(problem, domain));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, domain));
}

void _preconditionThatCannotBeSolved()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_hasQrCode + "\n" +
                                                     _fact_hasCheckInPasword + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkInWithQrCode, cp::Action(_condition_fromStr(_fact_hasQrCode, ontology),
                                                        _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_checkInWithPassword, cp::Action(_condition_fromStr(_fact_hasCheckInPasword, ontology),
                                                          _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _preferInContext()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_hasQrCode + "\n" +
                                                     _fact_hasCheckInPasword + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkInWithQrCode, cp::Action({},
                                                        _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                        _condition_fromStr(_fact_hasQrCode, ontology)));
  actions.emplace(_action_checkInWithPassword, cp::Action({},
                                                          _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                          _condition_fromStr(_fact_hasCheckInPasword, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack, ontology);
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack, ontology);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  _setFacts(problem.worldState, {}, problem.goalStack, ontology);
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack, ontology);
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack, ontology);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  actions.emplace(_action_checkInWithRealPerson, cp::Action({},
                                                            _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                            _condition_fromStr("!" + _fact_hasQrCode, ontology)));
  _setFacts(problem.worldState, {}, problem.goalStack, ontology);
  assert_eq(_action_checkInWithRealPerson + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));
}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_beginOfConversation + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_presented, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology),
                                            _condition_fromStr(_fact_beginOfConversation, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setFacts(problem.worldState, {_fact_beginOfConversation}, problem.goalStack, ontology);
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology));
}


void _avoidToDo2TimesTheSameActionIfPossble()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_presented, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology)));
  actions.emplace(_action_presentation, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology));

  _setGoalsForAPriority(problem, {_fact_greeted}, ontology);
  assert_eq(_action_greet, _solveStr(problem, actions, ontology));

  _setFacts(problem.worldState, {}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology));
}


void _takeHistoricalIntoAccount()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_presented, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology, &problem.historical));

  assert_eq(_action_presentation + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, ontology, &problem.historical));
}


void _goDoTheActionThatHaveTheMostPreferInContextValidated()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_advertised + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_is_close, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, _worldStateModification_fromStr(_fact_advertised, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_is_close, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                              _condition_fromStr(_fact_is_close, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_advertised + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_is_close}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_checkIn, _lookForAnActionToDoStr(problem, domain));
}


void _checkNotInAPrecondition()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_greeted, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(_condition_fromStr("!" + _fact_checkedIn, ontology),
                                            _worldStateModification_fromStr(_fact_greeted, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted}, ontology);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  problem.worldState.modify(_worldStateModification_fromStr(_fact_checkedIn, ontology), problem.goalStack,
                            _emptySetOfEvents, {}, {}, _now);
  assert_eq(std::string(), _lookForAnActionToDoConstStr(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted, ontology.types);

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_greeted}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_greeted}, ontology);
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  cp::Domain domain;
  _lookForAnActionToDo(problem, domain);
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());
}


void _checkActionHasAFact()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "\n" +
                                                     _fact_f + "\n" +
                                                     _fact_g, ontology.types);

  cp::ProblemModification effect(_worldStateModification_fromStr(_fact_a + " & !" + _fact_b, ontology));
  effect.potentialWorldStateModification = _worldStateModification_fromStr(_fact_c, ontology);
  effect.goalsToAdd[cp::GoalStack::defaultPriority] = {_goal(_fact_d, ontology)};
  const cp::Action action(_condition_fromStr(_fact_e, ontology),
                          effect,
                          _condition_fromStr(_fact_f, ontology));
  assert_true(action.hasFact(_fact(_fact_a, ontology)));
  assert_true(action.hasFact(_fact(_fact_b, ontology)));
  assert_true(action.hasFact(_fact(_fact_c, ontology)));
  assert_true(action.hasFact(_fact(_fact_d, ontology)));
  assert_true(action.hasFact(_fact(_fact_e, ontology)));
  assert_true(action.hasFact(_fact(_fact_f, ontology)));
  assert_false(action.hasFact(_fact(_fact_g, ontology)));
}


void _precoditionEqualEffect()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_beHappy, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_beHappy, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _addGoalEvenForEmptyAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  cp::Action act1Obj({}, {});
  act1Obj.effect.goalsToAddInCurrentPriority.push_back(_goal(_fact_a, ontology));
  actions.emplace(action1, act1Obj);
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  assert_true(problem.goalStack.goals().empty());
  cp::notifyActionDone(problem, domain, cp::ActionInvocationWithGoal(action1, {}, {}, 0), {});
  assert_false(problem.goalStack.goals().empty());
}

void _circularDependencies()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_hasCheckInPasword + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace("check-in-pwd", cp::Action(_condition_fromStr(_fact_hasCheckInPasword, ontology),
                                             _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace("inverse-of-check-in-pwd", cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                                        _worldStateModification_fromStr(_fact_hasCheckInPasword, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_beSad + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action(_condition_fromStr(_fact_beSad, ontology),
                                           _worldStateModification_fromStr("!" + _fact_beSad, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr("!" + _fact_beSad, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Historical historical;
  cp::Problem problem;
  _addFact(problem.worldState, _fact_beSad, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_beHappy}, ontology);
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology, _now, &historical));
}


void _actionWithConstantValue()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location");
  ontology.constants = cp::SetOfEntities::fromStr("kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("place - location", ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, _worldStateModification_fromStr("place=kitchen", ontology));
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen", ontology)});
  assert_eq(_action_navigate, _solveStr(problem, actions, ontology));
}


void _actionWithParameterizedValue()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location");
  ontology.constants = cp::SetOfEntities::fromStr("kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("place - location", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?target - location", ontology));
  cp::Action navigate({}, _worldStateModification_fromStr("place=?target", ontology, parameters));
  navigate.parameters = std::move(parameters);
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen", ontology)});
  assert_eq(_action_navigate + "(?target -> kitchen)", _solveStr(problem, actions, ontology));
}


void _actionWithParameterizedParameter()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("human");
  ontology.constants = cp::SetOfEntities::fromStr("h1 - human", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("isHappy(?h - human)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human - human", ontology));
  cp::Action joke({}, _worldStateModification_fromStr("isHappy(?human)", ontology, parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("isHappy(h1)", ontology)});
  assert_eq(_action_joke + "(?human -> h1)", _solveStr(problem, actions, ontology));
}


void _actionWithParametersInPreconditionsAndEffectsWithoutSolution()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("human");
  ontology.constants = cp::SetOfEntities::fromStr("h1 h2 - human", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("isEngaged(?h - human)\n"
                                                     "isHappy(?h - human)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human - human", ontology));
  cp::Action joke(_condition_fromStr("isEngaged(?human)", ontology, parameters),
                  _worldStateModification_fromStr("isHappy(?human)", ontology, parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _addFact(problem.worldState, "isEngaged(h2)", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal("isHappy(h1)", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
}

void _actionWithParametersInsideThePath()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location");
  ontology.constants = cp::SetOfEntities::fromStr("entrance kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("place - location\n"
                                                     "welcomePeople", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navigateParameters(1, _parameter("?target - location", ontology));
  cp::Action navigateAction({},
                            _worldStateModification_fromStr("place=?target", ontology, navigateParameters));
  navigateAction.parameters = std::move(navigateParameters);
  actions.emplace(_action_navigate, navigateAction);

  actions.emplace(_action_welcome,
                  cp::Action(_condition_fromStr("place=entrance", ontology),
                             _worldStateModification_fromStr("welcomePeople", ontology)));

  cp::Problem problem;
  _addFact(problem.worldState, "place=kitchen", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal("welcomePeople", ontology)});
  assert_eq<std::string>(_action_navigate + "(?target -> entrance)" + _sep +
                         _action_welcome, _solveStr(problem, actions, ontology));
  assert_true(_hasFact(problem.worldState, "place=entrance", ontology));
  assert_false(_hasFact(problem.worldState, "place=kitchen", ontology));
}


void _testPersistGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("welcomePeople", ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_welcome, cp::Action({}, _worldStateModification_fromStr("welcomePeople", ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("welcomePeople", ontology)});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions, ontology));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());

  problem = cp::Problem();
  _setGoalsForAPriority(problem, {_goal("persist(welcomePeople)", ontology)});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions, ontology));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
}


void _testPersistImplyGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_greeted + ", " + _fact_checkedIn + "))", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
  _addFact(problem.worldState, _fact_greeted, problem.goalStack, ontology);
  assert_eq<std::string>(_action_checkIn, _solveStr(problem, actions, ontology));
}


void _testImplyGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_greeted + ", " + _fact_checkedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
  // It is not a persistent goal it is removed
  _addFact(problem.worldState, _fact_greeted, problem.goalStack, ontology);
  assert_eq<std::string>("", _solveStr(problem, actions, ontology));
}


void _checkPreviousBugAboutSelectingAnInappropriateAction()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_engagedWithUser + "\n" +
                                                     _fact_userSatisfied + "\n" +
                                                     _fact_robotLearntABehavior + "\n" +
                                                     _fact_advertised, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action(_condition_fromStr(_fact_engagedWithUser, ontology),
                                                   _worldStateModification_fromStr(_fact_userSatisfied, ontology),
                                                   _condition_fromStr("!" + _fact_robotLearntABehavior, ontology)));
  actions.emplace(_action_checkIn, cp::Action({},
                                              _worldStateModification_fromStr("!" + _fact_robotLearntABehavior + " & " + _fact_advertised, ontology)));
  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {"persist(" + _fact_userSatisfied + ")"}, ontology);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions, ontology));
  _removeFact(problem.worldState, _fact_userSatisfied, problem.goalStack, ontology);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions, ontology));
}


void _dontLinkActionWithPreferredInContext()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_engagedWithUser + "\n" +
                                                     _fact_userSatisfied + "\n" +
                                                     _fact_checkedIn, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_userSatisfied, ontology),
                                                   _condition_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_engagedWithUser, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_userSatisfied}, ontology);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions, ontology));
}


void _checkPriorities()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_checkedIn, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_fact_greeted}}, {9, {_fact_beHappy}}}, ontology);
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology));
}


void _stackablePropertyOfGoals()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_presented, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted, ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology));

  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted, ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  problem2.goalStack.pushFrontGoal(_goal(_fact_presented, ontology), problem2.worldState, {}, 10);
  assert_eq(_action_presentation + _sep +
            _action_goodBoy, _solveStr(problem2, actions, ontology));
}



void _doNotRemoveAGoalWithMaxTimeToKeepInactiveEqual0BelowAGoalWithACondotionNotSatisfied()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_beHappy + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_presented, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));

  // Even if _fact_checkedIn has maxTimeToKeepInactive equal to 0, it is not removed because the goal with a higher priority is inactive.
  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions, ontology));

  cp::Problem problem2;
  _addFact(problem2.worldState, _fact_presented, problem2.goalStack, ontology); // The difference here is that the condition of the first goal is satisfied
  _setGoals(problem2, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem2, actions, ontology));


  cp::Problem problem3;
  _setGoals(problem3, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  _addFact(problem3.worldState, _fact_presented, problem3.goalStack, ontology); // The difference here is that the condition is validated after the add of the goal
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem3, actions, ontology));


  cp::Problem problem4;
  _setGoals(problem4, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  _addFact(problem4.worldState, _fact_presented, problem4.goalStack, ontology); // Here _fact_checkedIn goal shoud be removed from the stack
  _removeFact(problem4.worldState, _fact_presented, problem4.goalStack, ontology); // The difference here is that the condition was validated only punctually
  assert_eq(_action_goodBoy, _solveStr(problem4, actions, ontology));
}



void _checkMaxTimeToKeepInactiveForGoals()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn, ontology.types);

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));


  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted, ontology), _goal(_fact_checkedIn, ontology, 60)}}}, now);
  assert_eq(_action_greet + _sep +
            _action_checkIn, _solveStr(problem, actions, ontology, now));


  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted, ontology), _goal(_fact_checkedIn, ontology, 60)}}}, now);
  now = std::make_unique<std::chrono::steady_clock::time_point>(*now + std::chrono::seconds(100));
  assert_eq(_action_greet, _solveStr(problem2, actions, ontology, now));
}



void _changePriorityOfGoal()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_userSatisfied, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  std::map<int, std::vector<cp::Goal>> goalsFromSubscription;
  cp::Problem problem;
  auto onGoalsChangedConnection = problem.goalStack.onGoalsChanged.connectUnsafe([&](const std::map<int, std::vector<cp::Goal>>& pGoals) {
    goalsFromSubscription = pGoals;
  });
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, ontology);
  {
    auto& goals = problem.goalStack.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(1, goals.find(9)->second.size());
    assert_eq<std::size_t>(2, goals.find(10)->second.size());
  }

  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, true, problem.worldState, _now);
  {
    auto& goals = problem.goalStack.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(2, goals.find(9)->second.size());
    assert_eq(_fact_checkedIn, goals.find(9)->second[0].toStr());
    assert_eq(_fact_userSatisfied, goals.find(9)->second[1].toStr());
    assert_eq<std::size_t>(1, goals.find(10)->second.size());
  }

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, ontology);
  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, false, problem.worldState, _now);
  {
    auto& goals = problem.goalStack.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(2, goals.find(9)->second.size());
    assert_eq(_fact_userSatisfied, goals.find(9)->second[0].toStr());
    assert_eq(_fact_checkedIn, goals.find(9)->second[1].toStr());
    assert_eq<std::size_t>(1, goals.find(10)->second.size());
  }

  _setGoals(problem, {{10, {_fact_greeted, _fact_checkedIn}}}, ontology);
  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, true, problem.worldState, _now);
  {
    auto& goals = problem.goalStack.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_eq<std::size_t>(1, goalsRemoved.size());
    assert_eq(_fact_userSatisfied, *goalsRemoved.begin());
    assert_eq<std::size_t>(1, goals.find(9)->second.size());
    assert_eq(_fact_checkedIn, goals.find(9)->second[0].toStr());
    assert_eq<std::size_t>(1, goals.find(10)->second.size());
  }
  onGoalsRemovedConnection.disconnect();
  onGoalsChangedConnection.disconnect();
}


void _factChangedNotification()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_greeted + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_userSatisfied + "\n" +
                                                     _fact_punctual_p1 + "\n" +
                                                     _fact_beginOfConversation, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn + "&" + _fact_punctual_p1, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  std::set<cp::Fact> factsChangedFromSubscription;
  cp::Problem problem;
  _addFact(problem.worldState, _fact_beginOfConversation, problem.goalStack, ontology);
  auto factsChangedConnection = problem.worldState.onFactsChanged.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsChangedFromSubscription = pFacts;
  });
  std::list<cp::Fact> punctualFactsAdded;
  auto onPunctualFactsConnection = problem.worldState.onPunctualFacts.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    punctualFactsAdded.insert(punctualFactsAdded.end(), pFacts.begin(), pFacts.end());
  });
  std::list<cp::Fact> factsAdded;
  auto onFactsAddedConnection = problem.worldState.onFactsAdded.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsAdded.insert(factsAdded.end(), pFacts.begin(), pFacts.end());
  });
  std::list<cp::Fact> factsRemoved;
  auto onFactsRemovedConnection = problem.worldState.onFactsRemoved.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsRemoved.insert(factsRemoved.end(), pFacts.begin(), pFacts.end());
  });

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, ontology);
  assert_eq({}, factsChangedFromSubscription);

  auto plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_greet, plannerResult.actionInvocation.actionId);
  assert_eq({_fact(_fact_beginOfConversation, ontology), _fact(_fact_greeted, ontology)}, factsChangedFromSubscription);
  assert_eq({}, punctualFactsAdded);
  assert_eq({_fact(_fact_greeted, ontology)}, factsAdded);
  factsAdded.clear();
  assert_eq({}, factsRemoved);

  plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_checkIn, plannerResult.actionInvocation.actionId);
  assert_eq({_fact(_fact_beginOfConversation, ontology), _fact(_fact_greeted, ontology), _fact(_fact_checkedIn, ontology)}, factsChangedFromSubscription);
  assert_eq({_fact(_fact_punctual_p1, ontology)}, punctualFactsAdded);
  assert_eq({_fact(_fact_checkedIn, ontology)}, factsAdded);
  assert_eq({}, factsRemoved);
  _removeFact(problem.worldState, _fact_greeted, problem.goalStack, ontology);
  assert_eq({_fact(_fact_beginOfConversation, ontology), _fact(_fact_checkedIn, ontology)}, factsChangedFromSubscription);
  assert_eq({_fact(_fact_punctual_p1, ontology)}, punctualFactsAdded);
  assert_eq({_fact(_fact_checkedIn, ontology)}, factsAdded);
  assert_eq({_fact(_fact_greeted, ontology)}, factsRemoved);

  onFactsRemovedConnection.disconnect();
  onPunctualFactsConnection.disconnect();
  onFactsAddedConnection.disconnect();
  factsChangedConnection.disconnect();
}




void _checkEvents()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_headTouched + "\n" +
                                                     _fact_checkedIn, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  // Event: if (_fact_headTouched) then remove(_fact_headTouched) and addGoal(_fact_checkedIn)
  domain.addSetOfEvents(cp::Event(_condition_fromStr(_fact_headTouched, ontology),
                                  _worldStateModification_fromStr("!" + _fact_headTouched, ontology),
                                  _emptyParameters, {{{9, {_goal(_fact_checkedIn, ontology)}}}}));
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfEventsMap = domain.getSetOfEvents();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched, ontology)); // removed because of the event
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}



void _checkEventsWithImply()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_headTouched + "\n" +
                                                     _fact_checkedIn + "\n" +
                                                     _fact_userWantsToCheckedIn, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  // Event: if (_fact_headTouched) then add(_fact_userWantsToCheckedIn) and remove(_fact_headTouched)
  cp::Domain domain(std::move(actions), ontology,
                    cp::Event(_condition_fromStr(_fact_headTouched, ontology),
                              _worldStateModification_fromStr(_fact_userWantsToCheckedIn + " & !" + _fact_headTouched, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_userWantsToCheckedIn + ", " + _fact_checkedIn + "))", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfEvents();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, ontology, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched, ontology)); // removed because of the event
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkEventWithPunctualCondition()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_punctual_headTouched + "\n" +
                                                     _fact_userWantsToCheckedIn, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn, ontology)));

  // Event: if (_fact_punctual_headTouched) then add(_fact_userWantsToCheckedIn)
  cp::Domain domain(std::move(actions), ontology,
                    cp::Event(_condition_fromStr(_fact_punctual_headTouched, ontology),
                              _worldStateModification_fromStr(_fact_userWantsToCheckedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfEvents();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, ontology, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched, ontology)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkEventAtEndOfAPlan()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_punctual_headTouched + "\n" +
                                                     _fact_userWantsToCheckedIn + "\n" +
                                                     _fact_punctual_checkedIn, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_punctual_checkedIn, ontology)));

  cp::SetOfEvents setOfEvents;
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_punctual_headTouched, ontology),
                            _worldStateModification_fromStr(_fact_userWantsToCheckedIn, ontology)));
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_punctual_checkedIn, ontology),
                            _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn, ontology)));
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfEvents();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, ontology, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched, ontology)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkEventInsideAPlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_c, ontology), _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  {
    cp::SetOfEvents setOfEvents;
    setOfEvents.add(cp::Event(_condition_fromStr(_fact_a, ontology),
                              _worldStateModification_fromStr(_fact_b, ontology)));
    setOfEvents.add(cp::Event(_condition_fromStr(_fact_b + "&" + _fact_d, ontology),
                              _worldStateModification_fromStr(_fact_c, ontology)));
    domain.addSetOfEvents(std::move(setOfEvents));
  }

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  assert_eq<std::string>("", _solveStrConst(problem, domain));

  domain.addSetOfEvents(cp::Event(_condition_fromStr(_fact_b, ontology),
                                  _worldStateModification_fromStr(_fact_c, ontology)));

  assert_eq(action1 + _sep + action2, _solveStrConst(problem, domain)); // check with a copy of the problem
  assert_true(!_hasFact(problem.worldState, _fact_a, ontology));
  assert_true(!_hasFact(problem.worldState, _fact_b, ontology));
  assert_true(!_hasFact(problem.worldState, _fact_c, ontology));
  assert_true(!_hasFact(problem.worldState, _fact_d, ontology));
  assert_eq(action1 + _sep + action2, _solveStr(problem, domain));
  assert_true(_hasFact(problem.worldState, _fact_a, ontology));
  assert_true(_hasFact(problem.worldState, _fact_b, ontology));
  assert_true(_hasFact(problem.worldState, _fact_c, ontology));
  assert_true(_hasFact(problem.worldState, _fact_d, ontology));
}


void _checkEventThatAddAGoal()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";
  const std::string action5 = "action5";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "\n" +
                                                     _fact_f + "\n" +
                                                     _fact_g, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_c, ontology),
                                      _worldStateModification_fromStr(_fact_d, ontology)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_c + "&" + _fact_f, ontology),
                                      _worldStateModification_fromStr(_fact_e, ontology)));
  actions.emplace(action4, cp::Action(_condition_fromStr(_fact_b, ontology),
                                      _worldStateModification_fromStr(_fact_f, ontology)));
  actions.emplace(action5, cp::Action(_condition_fromStr(_fact_b, ontology),
                                      _worldStateModification_fromStr(_fact_g, ontology)));
  cp::SetOfEvents setOfEvents;
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_a, ontology),
                            _worldStateModification_fromStr(_fact_b, ontology),
                            _emptyParameters, {{cp::GoalStack::defaultPriority, {_goal(_fact_e, ontology)}}}));
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_b, ontology),
                            _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_g + ", " + _fact_d + ")", ontology)});
  assert_eq<std::string>("", _solveStrConst(problem, domain));
  auto& setOfEventsMap = domain.getSetOfEvents();
  _addFact(problem.worldState, _fact_g, problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + _sep + action4 + _sep + action3 + _sep + action2, _solveStr(problem, domain));
}


void _testGetNotSatisfiedGoals()
{
  auto goal1 = "persist(!" + _fact_a + ")";
  auto goal2 = "persist(" + _fact_b + ")";
  auto goal3 = "imply(" + _fact_c + ", " + _fact_d + ")";
  auto goal4 = "persist(imply(!" + _fact_c + ", " + _fact_d + "))";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d, ontology.types);

  cp::Problem problem;
  _addGoalsForAPriority(problem, {goal1}, ontology, {}, cp::GoalStack::defaultPriority + 1);
  _addGoalsForAPriority(problem, {goal2, goal3, goal4}, ontology);

  assert_eq(goal1 + ", " + goal2 + ", " + goal3 + ", " + goal4, cp::printGoals(problem.goalStack.goals()));
  assert_eq(goal2 + ", " + goal4, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_a, problem.goalStack, ontology);
  assert_eq(goal1 + ", " + goal2 + ", " + goal4, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_c, problem.goalStack, ontology);
  assert_eq(goal1 + ", " + goal2 + ", " + goal3, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_d, problem.goalStack, ontology);
  assert_eq(goal1 + ", " + goal2, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _removeFact(problem.worldState, _fact_a, problem.goalStack, ontology);
  assert_eq(goal2, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_b, problem.goalStack, ontology);
  assert_eq<std::string>("", cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _removeFact(problem.worldState, _fact_d, problem.goalStack, ontology);
  assert_eq(goal3, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
}



void _testGoalUnderPersist()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, ontology, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, ontology, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, ontology, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    _addFact(problem.worldState, _fact_a, problem.goalStack, ontology);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, ontology, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, ontology, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {_fact_c}, ontology, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.goalStack.pushBackGoal({_goal("persist(!" + _fact_e + ")", ontology)}, problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    problem.goalStack.pushBackGoal(_goal(_fact_c, ontology), problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, now);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 1)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));

    _removeFact(problem.worldState, _fact_c, problem.goalStack, ontology);
    problem.goalStack.pushBackGoal(_goal(_fact_c, ontology), problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    auto plannerResult = _lookForAnActionToDo(problem, domain, now);
    assert_eq(action2, plannerResult.actionInvocation.actionId);

    now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now() + std::chrono::minutes(5));
    cp::notifyActionDone(problem, domain, plannerResult, now);

    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, now);
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId); // Not action1 because it was inactive for too long
  }

}


void _checkLinkedEvents()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_userWantsToCheckedIn + "\n" +
                                                     _fact_a + "\n" +
                                                     _fact_punctual_p1 + "\n" +
                                                     _fact_punctual_p2 + "\n" +
                                                     _fact_punctual_p3 + "\n" +
                                                     _fact_punctual_p4 + "\n" +
                                                     _fact_punctual_p5 + "\n" +
                                                     _fact_e, ontology.types);

  cp::SetOfEvents setOfEvents;
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_punctual_p2, ontology),
                            _worldStateModification_fromStr(_fact_a, ontology)));
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_punctual_p5, ontology),
                            _worldStateModification_fromStr(_fact_punctual_p2 + "&" + _fact_punctual_p3, ontology)));
  setOfEvents.add(cp::Event(_condition_fromStr(_fact_punctual_p4, ontology),
                            _worldStateModification_fromStr(_fact_punctual_p5 + "&" + _fact_punctual_p1, ontology)));

  std::map<cp::SetOfEventsId, cp::SetOfEvents> setOfEventsMap = {{"soe", setOfEvents}};
  assert_false(_hasFact(problem.worldState, _fact_a, ontology));
  _addFact(problem.worldState, _fact_punctual_p4, problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_a, ontology));
}



void _oneStepTowards()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_checkedIn + "\n" +
                                                     _fact_greeted + "\n" +
                                                     _fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_beHappy, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification greetPbModification;
  greetPbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_greeted, ontology);
  actions.emplace(_action_greet, cp::Action({}, greetPbModification));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  static const std::string actionb = "actionb";
  actions.emplace(actionb, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  auto implyGoal = _goal("persist(imply(" + _fact_a + ", " + _fact_b + "))", ontology, 0);
  _setGoals(problem, {{11, {implyGoal}},
                      {10, {_goal("oneStepTowards(" + _fact_greeted + ")", ontology, 0)}},
                      {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}}, _now);
  problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, _now);
  assert_eq(_action_greet, _lookForAnActionToDoStr(problem, domain, _now));
  assert_eq(_action_greet, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, _now));
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, _now));
  _addFact(problem.worldState, _fact_a, problem.goalStack, ontology);
  assert_eq(actionb, _lookForAnActionToDoStr(problem, domain, _now));
  assert_eq<std::string>(actionb + _sep + _action_goodBoy, _solveStrConst(problem, domain));
  assert_eq<std::string>(implyGoal.toStr() + _sep + _fact_beHappy, _getGoalsDoneDuringAPlannificationConst(problem, domain));
}


void _infrenceLinksFromManyEventsSets()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_punctual_p2 + "\n" +
                                                     _fact_punctual_p1, ontology.types);

  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification pbModification;
  pbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_d, ontology);
  actions.emplace(action1, cp::Action({}, pbModification));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  assert_true(cp::GoalStack::defaultPriority >= 1);
  auto lowPriority = cp::GoalStack::defaultPriority - 1;
  domain.addSetOfEvents(cp::Event(_condition_fromStr(_fact_punctual_p2, ontology),
                                  {}, _emptyParameters,
                                  {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")", ontology)}}}));
  cp::Problem problem;
  _setGoals(problem, {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")", ontology, 0)}}});

  {
    cp::SetOfEvents setOfEvents2;
    setOfEvents2.add(cp::Event(_condition_fromStr(_fact_punctual_p1, ontology),
                               _worldStateModification_fromStr(_fact_b + "&" + _fact_punctual_p2, ontology)));
    setOfEvents2.add(cp::Event(_condition_fromStr(_fact_b, ontology),
                               {}, _emptyParameters,
                               {{cp::GoalStack::defaultPriority, {_goal("oneStepTowards(" + _fact_c + ")", ontology)}}}));
    domain.addSetOfEvents(setOfEvents2);
  }

  auto& setOfEventsMap = domain.getSetOfEvents();
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  _addFact(problem.worldState, _fact_punctual_p1, problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
}


void _factValueModification()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("a b - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("persist(imply(" + _fact_a + "=a, " + "!" + _fact_b + "))", ontology, 0)}}});

  _addFact(problem.worldState, _fact_b, problem.goalStack, ontology);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, _now));
  _addFact(problem.worldState, _fact_a + "=a", problem.goalStack, ontology);
  assert_eq(action1, _lookForAnActionToDoStr(problem, domain, _now));
  _addFact(problem.worldState, _fact_a + "=b", problem.goalStack, ontology);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, _now));
}


void _removeGoaWhenAnActionFinishesByAddingNewGoals()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification wm(_worldStateModification_fromStr(_fact_a, ontology));
  wm.goalsToAddInCurrentPriority.push_back(_goal(_fact_b, ontology, 0));
  actions.emplace(action1, cp::Action({}, wm));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  _setGoals(problem, {{27, {_fact_a}}}, ontology, _now);

  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_a, *goalsRemoved.begin());
  onGoalsRemovedConnection.disconnect();

  // The only remaning goal is the goal we just added
  assert_eq<std::size_t>(1u, problem.goalStack.goals().size());
  assert_eq(27, problem.goalStack.goals().begin()->first);
  assert_eq<std::size_t>(1u, problem.goalStack.goals().begin()->second.size());
  assert_eq(_fact_b, problem.goalStack.goals().begin()->second.begin()->toStr());
}


void _setWsModification()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me object - entity\n"
                                                  "corridor kitchen - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?o - entity) - loc_type", ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("set(location(me), location(object))", ontology));
  actions.emplace(_action_navigate, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack, ontology);
  _addFact(problem.worldState, "location(object)=kitchen", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal("location(me)=kitchen", ontology)});
  assert_eq(_action_navigate, _solveStr(problem, actions, ontology));
}


void _forAllWsModification()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me object1 object2 - entity\n"
                                                  "corridor kitchen - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?o - entity) - loc_type\n"
                                                     "grab(?e - entity, ?o - entity)", ontology.types);

  const std::string action1 = "action1";
  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("forAll(?obj - entity, grab(me, ?obj), set(location(?obj), location(me)))", ontology));
  actions.emplace(action1, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack, ontology);
  _addFact(problem.worldState, "location(object1)=kitchen", problem.goalStack, ontology);
  _addFact(problem.worldState, "grab(me, object1)", problem.goalStack, ontology);
  _addFact(problem.worldState, "grab(me, object2)", problem.goalStack, ontology);

  _setGoalsForAPriority(problem, {_goal("location(object2)=corridor", ontology)});
  assert_eq(action1, _solveStr(problem, actions, ontology));
}


void _actionNavigationAndGrabObjectWithParameters()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me sweets - entity\n"
                                                  "corridor kitchen - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?o - entity) - loc_type\n"
                                                     "grab(?e - entity, ?o - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation - loc_type", ontology));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object - entity", ontology));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack, ontology);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack, ontology);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)=*", ontology))->value);
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)",
                         _solveStr(problem, actions, ontology));
}


void _actionNavigationAndGrabObjectWithParameters2()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me sweets - entity\n"
                                                  "corridor kitchen - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?o - entity) - loc_type\n"
                                                     "grab(?e - entity, ?o - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation - loc_type", ontology));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object - entity", ontology));
  cp::Action grabAction(_condition_fromStr("equals(location(?object), location(me))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack, ontology);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack, ontology);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)=*", ontology))->value);
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)",
                         _solveStr(problem, actions, ontology));
}



void _moveObject()
{
  const std::string actionNavigate2 = "actionNavigate2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me sweets - entity\n"
                                                  "corridor kitchen bedroom - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("location(?o - entity) - loc_type\n"
                                                     "grab(?e - entity, ?o - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation - loc_type", ontology));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> nav2Parameters{_parameter("?targetLocation - loc_type", ontology), _parameter("?object - entity", ontology)};
  cp::Action navAction2(_condition_fromStr("grab(me, ?object)", ontology, nav2Parameters),
                        _worldStateModification_fromStr("location(me)=?targetLocation & location(?object)=?targetLocation", ontology, nav2Parameters));
  navAction2.parameters = std::move(nav2Parameters);
  actions.emplace(actionNavigate2, navAction2);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object - entity", ontology));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack, ontology);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal("location(sweets)=bedroom", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets), " +
                         actionNavigate2 + "(?object -> sweets, ?targetLocation -> bedroom)",
                         _solveStr(problem, actions, ontology));
}


void _moveAndUngrabObject()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "robot\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "sweets - entity\n"
                                                  "kitchen bedroom - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("locationOfRobot(?r - robot) - loc_type\n"
                                                     "locationOfObj(?o - entity) - loc_type\n"
                                                     "grab(?e - robot, ?o - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation - loc_type", ontology));
  cp::Action navAction({}, _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object - entity", ontology));
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters(1, _parameter("?object - entity", ontology));
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::Problem problem;
  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> eventParameters{_parameter("?targetLocation - loc_type", ontology), _parameter("?object - entity", ontology)};
  cp::Event event(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", ontology, eventParameters),
                  _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", ontology, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  auto& setOfEventsMap = domain.getSetOfEvents();
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)", ontology)});

  assert_eq(_action_navigate + "(?targetLocation -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "locationOfObj(sweets)=kitchen", ontology));
}


void _failToMoveAnUnknownObject()
{
  const std::string actionWhereIsObject = "actionWhereIsObject";
  const std::string actionLeavePod = "actionLeavePod";
  const std::string actionRanomWalk = "actionRanomWalk";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "robot\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "sweets - entity\n"
                                                  "kitchen bedroom - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("locationOfRobot(?r - robot) - loc_type\n"
                                                     "locationOfObj(?o - entity) - loc_type\n"
                                                     "grab(?e - robot, ?o - entity)\n"
                                                     "charging(?r - robot)\n"
                                                     "isLost", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation - loc_type", ontology));
  cp::Action navAction(_condition_fromStr("!isLost & !charging(me)", ontology, navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  cp::Action randomWalkAction(_condition_fromStr("!charging(me)", ontology),
                              _worldStateModification_fromStr("!isLost", ontology));
  actions.emplace(actionRanomWalk, randomWalkAction);

  cp::Action leavePodAction({}, _worldStateModification_fromStr("!charging(me)", ontology));
  actions.emplace(actionLeavePod, leavePodAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object - entity", ontology), _parameter("?aLocation - loc_type", ontology)};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObj(?object)=*", ontology, whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObj(?object)=?aLocation", ontology, whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object - entity", ontology)};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object - entity", ontology)};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> eventParameters{_parameter("?targetLocation - loc_type", ontology), _parameter("?object - entity", ontology)};
  cp::Event event(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", ontology, eventParameters),
                  _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", ontology, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)", ontology)});

  auto& setOfEventsMap = domain.getSetOfEvents();
  _addFact(problem.worldState, "charging(me)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)", ontology)});
  assert_eq(actionLeavePod, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _completeMovingObjectScenario()
{
  const std::string actionWhereIsObject = "actionWhereIsObject";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "robot\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "sweets bottle - entity\n"
                                                  "kitchen bedroom entrance - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("locationOfRobot(?r - robot) - loc_type\n"
                                                     "locationOfObject(?o - entity) - loc_type\n"
                                                     "grab(?e - robot) - entity\n"
                                                     "charging(?r - robot)\n"
                                                     "isLost\n"
                                                     "pathIsBlocked\n"
                                                     "lost(?r - robot)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters{_parameter("?targetPlace - loc_type", ontology)};
  cp::Action navAction(_condition_fromStr("!lost(me) & !pathIsBlocked", ontology, navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetPlace", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object - entity", ontology)};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObject(?object)) & not(exists(?o - entity, grab(me)=?o))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me)=?object", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object - entity", ontology)};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me)=?object", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object - entity", ontology), _parameter("?aLocation - loc_type", ontology)};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObject(?object)=*", ontology, whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObject(?object)=?aLocation", ontology, whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> eventParameters{_parameter("?object - entity", ontology), _parameter("?location - loc_type", ontology)};
  cp::Event event(_condition_fromStr("locationOfRobot(me)=?location & grab(me)=?object", ontology, eventParameters),
                  _worldStateModification_fromStr("locationOfObject(?object)=?location", ontology, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets", ontology)});

  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  auto& setOfEventsMap = domain.getSetOfEvents();
  _addFact(problem.worldState, "locationOfObject(sweets)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets", ontology)});
  assert_eq(_action_navigate + "(?targetPlace -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetPlace -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal("locationOfObject(bottle)=entrance & !grab(me)=bottle", ontology)});
  assert_eq(actionWhereIsObject + "(?aLocation -> entrance, ?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}

void _eventWithANegatedFactWithParameter()
{
  const std::string actionUngrabLeftHand = "actionUngrabLeftHand";
  const std::string actionUngrabRightHand = "actionUngrabRightHand";
  const std::string actionUngrabBothHands = "actionUngrabBothHands";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "robot\n"
                                           "entity");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "sweets bottle glass - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("grab(?e - robot, ?e - entity)\n"
                                                     "hasTwoHandles(?e - entity)\n"
                                                     "grabLeftHand(?r - robot) - entity\n"
                                                     "grabRightHand(?r - robot) - entity", ontology.types);
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> ungrabLeftParameters{_parameter("?object - entity", ontology)};
  cp::Action ungrabLeftAction(_condition_fromStr("!hasTwoHandles(?object)", ontology, ungrabLeftParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object", ontology, ungrabLeftParameters));
  ungrabLeftAction.parameters = std::move(ungrabLeftParameters);
  actions.emplace(actionUngrabLeftHand, ungrabLeftAction);

  std::vector<cp::Parameter> ungrabRightParameters{_parameter("?object - entity", ontology)};
  cp::Action ungrabRightAction(_condition_fromStr("!hasTwoHandles(?object)", ontology, ungrabRightParameters),
                               _worldStateModification_fromStr("!grabRightHand(me)=?object", ontology, ungrabRightParameters));
  ungrabRightAction.parameters = std::move(ungrabRightParameters);
  actions.emplace(actionUngrabRightHand, ungrabRightAction);

  std::vector<cp::Parameter> ungrabBothParameters{_parameter("?object - entity", ontology)};
  cp::Action ungrabBothAction(_condition_fromStr("hasTwoHandles(?object)", ontology, ungrabBothParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", ontology, ungrabBothParameters));
  ungrabBothAction.parameters = std::move(ungrabBothParameters);
  actions.emplace(actionUngrabBothHands, ungrabBothAction);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> eventParameters{_parameter("?object - entity", ontology)};
  cp::Event event(_condition_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", ontology, eventParameters),
                  _worldStateModification_fromStr("!grab(me, ?object)", ontology, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);

  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, "hasTwoHandles(sweets)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grabLeftHand(me)=sweets", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grabRightHand(me)=sweets", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grab(me, sweets)", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, sweets)", ontology)});

  assert_eq(actionUngrabBothHands + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, sweets)", ontology));

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, bottle)", ontology)});
  assert_eq(actionUngrabLeftHand + "(?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grabRightHand(me)=glass", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "grab(me, glass)", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, glass)", ontology)});
  assert_eq(actionUngrabRightHand + "(?object -> glass)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, glass)", ontology));
}


void _actionWithANegatedFactNotTriggeredIfNotNecessary()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr("!" + _fact_a, ontology),
                                      _condition_fromStr(_fact_c + " & " + _fact_d, ontology)));

  actions.emplace(action2, cp::Action(_condition_fromStr("!" + _fact_a + " & " + _fact_e, ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_e, ontology)));

  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  _addFact(problem.worldState, _fact_c, problem.goalStack, ontology);
  _addFact(problem.worldState, _fact_d, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _useTwoTimesAnEvent()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("obj1 obj2 - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "(?e - entity)\n" +
                                                     _fact_c + "(?e - entity)", ontology.types);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> eventParameters{_parameter("?object - entity", ontology)};
  cp::Event event(_condition_fromStr(_fact_a + " & " + _fact_b + "(?object)", ontology, eventParameters),
                  _worldStateModification_fromStr(_fact_c + "(?object)", ontology, eventParameters));
  event.parameters = std::move(eventParameters);
  setOfEvents.add(event);

  std::map<std::string, cp::Action> actions;
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj2)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_a, problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj1)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj2)", ontology));
}


void _linkWithAnyValueInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr("!" + _fact_a + "=*", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));

  std::vector<cp::Parameter> act2Parameters{_parameter("?aVal - entity", ontology)};
  cp::Action act2({}, _worldStateModification_fromStr("!" + _fact_a + "=?aVal", ontology, act2Parameters));
  act2.parameters = std::move(act2Parameters);
  actions.emplace(action2, act2);

  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "=toto", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  assert_eq(action2 + "(?aVal -> toto)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _removeAFactWithAnyValue()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + " - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + " & !" + _fact_b + "=*", ontology)));
  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "=toto", problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal(_fact_a, ontology)});
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_b + "=toto", ontology));
}



void _notDeducePathIfTheParametersOfAFactAreDifferents()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("e1 e2 - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?e - entity)\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d, ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "(e1)", ontology),
                                      _condition_fromStr(_fact_c, ontology)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "(e2)", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_b, ontology),
                                      _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "(e2)", problem.goalStack, ontology);
  _addFact(problem.worldState, _fact_c, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
}


void _checkPreferInContext()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology), _condition_fromStr(_fact_b, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_a}, ontology);
  assert_eq(action1, _solveStrConst(problem, actions, ontology, &problem.historical));
  assert_eq(action1, _solveStrConst(problem, actions, ontology, &problem.historical));
}


void _checkPreferHighImportanceOfNotRepeatingIt()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  auto action1Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology), _condition_fromStr(_fact_b, ontology));
  action1Obj.highImportanceOfNotRepeatingIt = true;
  actions.emplace(action1, action1Obj);
  auto action2Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology));
  action2Obj.highImportanceOfNotRepeatingIt = true;
  actions.emplace(action2, action2Obj);

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_fact_a}, ontology);
  assert_eq(action1, _solveStrConst(problem, actions, ontology, &problem.historical));
  assert_eq(action2, _solveStrConst(problem, actions, ontology, &problem.historical));
}


void _actionWithFactWithANegatedFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("a b - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "=a", ontology)));

  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "!=b & " + _fact_d, ontology),
                                      _worldStateModification_fromStr(_fact_c, ontology)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_d, ontology),
                                      _condition_fromStr(_fact_e, ontology)));

  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack, ontology);
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _negatedFactValueInWorldState()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("a b c - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a + "!=b", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  {
    cp::Problem problem;
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=b", problem.goalStack, ontology);
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=c", problem.goalStack, ontology);
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

    assert_eq<std::string>(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "!=b", problem.goalStack, ontology);
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

    assert_eq<std::string>(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }
}


void _problemThatUseADomainThatChangedSinceLastUsage()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a, ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions), ontology);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, _now)); // set problem cache about domain

  domain.addAction(action2, cp::Action({},
                                       _worldStateModification_fromStr(_fact_a, ontology)));

  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  assert_eq<std::string>(action2, _lookForAnActionToDoStr(problem, domain, _now)); // as domain as changed since last time the problem cache should be regenerated
}


void _checkFilterFactInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location");
  ontology.constants = cp::SetOfEntities::fromStr("obj1 obj2 - entity\n"
                                                  "loc1 loc2 - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?e - entity)\n" +
                                                     _fact_b + "(?e - entity, ?l - location)\n" +
                                                     _fact_c, ontology.types);

  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj - entity", ontology), _parameter("?loc - location", ontology)};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "(?obj)", ontology, act2Parameters),
                        _worldStateModification_fromStr(_fact_c, ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);
  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj1, loc2)", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});
  assert_eq<std::string>(action1 + "(?obj -> obj1)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>(action2 + "(?loc -> loc1, ?obj -> obj1)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _checkFilterFactInConditionAndThenPropagate()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location");
  ontology.constants = cp::SetOfEntities::fromStr("obj1 obj2 - entity\n"
                                                  "loc1 loc2 - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?e - entity, ?l - location)\n" +
                                                     _fact_c + "(?l - location)\n" +
                                                     _fact_d + "(?l - location)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj - entity", ontology), _parameter("?loc - location", ontology)};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "=?obj", ontology, act2Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  std::vector<cp::Parameter> act3Parameters{_parameter("?loc - location", ontology)};
  cp::Action actionObj3(_condition_fromStr(_fact_c + "(?loc)", ontology, act3Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?loc)", ontology, act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj2, loc2)", problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(loc2)", ontology)});
  assert_eq(action1 + "(?obj -> obj2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?loc -> loc2, ?obj -> obj2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action3 + "(?loc -> loc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _checkOutputValueOfLookForAnActionToDo()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a, ontology)});

  {
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, _now, nullptr, &lookForAnActionOutputInfos);
    assert(!res.empty());
    assert_eq(cp::PlannerStepType::IN_PROGRESS, lookForAnActionOutputInfos.getType());
    assert_eq<std::size_t>(0, lookForAnActionOutputInfos.nbOfSatisfiedGoals());
  }

  {
    _addFact(problem.worldState, _fact_a, problem.goalStack, ontology, setOfEventsMap, _now);
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, _now, nullptr, &lookForAnActionOutputInfos);
    assert(res.empty());
    assert_eq(cp::PlannerStepType::FINISHED_ON_SUCCESS, lookForAnActionOutputInfos.getType());
    assert_eq<std::size_t>(1, lookForAnActionOutputInfos.nbOfSatisfiedGoals());
  }

  {
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, _now, nullptr, &lookForAnActionOutputInfos);
    assert(res.empty());
    assert_eq(cp::PlannerStepType::FINISEHD_ON_FAILURE, lookForAnActionOutputInfos.getType());
    assert_eq<std::size_t>(0, lookForAnActionOutputInfos.nbOfSatisfiedGoals());
  }
}


void _hardProblemThatNeedsToBeSmart()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";
  const std::string action5 = "action5";
  const std::string action6 = "action6";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("val1 val2 val3 val4 - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "\n" +
                                                     _fact_f, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + "=val1", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + "=val2", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action3, actionObj3);

  cp::Action actionObj4(_condition_fromStr(_fact_a + "=val3 & !" + _fact_c, ontology),
                        _worldStateModification_fromStr("!" + _fact_d + " & " + _fact_f, ontology));
  actions.emplace(action4, actionObj4);

  cp::Action actionObj5(_condition_fromStr(_fact_a + "=val4", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action5, actionObj5);

  cp::Action actionObj6(_condition_fromStr(_fact_b + " & !" + _fact_d, ontology),
                        _worldStateModification_fromStr(_fact_e, ontology));
  actions.emplace(action6, actionObj6);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal(_fact_e, ontology)});

  assert_eq(action1 + "(?obj -> val3)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action4, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> val1)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action6, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
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

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("val1 val2 val3 val4 - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "\n" +
                                                     _fact_f + "\n" +
                                                     _fact_g, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + "=val1", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + "=val2", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action3, actionObj3);

  cp::Action actionObj4(_condition_fromStr(_fact_a + "=val3 & !" + _fact_c, ontology),
                        _worldStateModification_fromStr("!" + _fact_d + " & " + _fact_f, ontology));
  actions.emplace(action4, actionObj4);

  cp::Action actionObj5(_condition_fromStr(_fact_a + "=val4", ontology),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c, ontology));
  actions.emplace(action5, actionObj5);

  cp::Action actionObj6(_condition_fromStr(_fact_b + " & !" + _fact_d + " & " + _fact_g, ontology),
                        _worldStateModification_fromStr(_fact_e, ontology));
  actions.emplace(action6, actionObj6);

  cp::Action actionObj7(_condition_fromStr(_fact_f, ontology),
                        _worldStateModification_fromStr(_fact_g, ontology));
  actions.emplace(action7, actionObj7);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, ontology, setOfEventsMap, _now);
  _setGoalsForAPriority(problem, {_goal(_fact_e, ontology)});

  assert_eq(action1 + "(?obj -> val3)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());

  assert_eq<std::string>(action1 + "(?obj -> val3)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_eq<std::string>(action4, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_eq<std::string>(action7 + _sep + action1 + "(?obj -> val1)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_eq<std::string>(action2, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_eq<std::string>(action6, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_eq<std::string>("", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
}



void _checkOverallEffectDuringParallelisation()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c + "\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e, ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("!" + _fact_d, ontology),
                        _worldStateModification_fromStr(_fact_a + " & !" + _fact_d, ontology));
  actionObj1.effect.worldStateModificationAtStart = _worldStateModification_fromStr(_fact_d + " & " + _fact_e, ontology);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr("!" + _fact_d, ontology),
                        _worldStateModification_fromStr(_fact_b + " & !" + _fact_d, ontology));
  actionObj2.effect.worldStateModificationAtStart = _worldStateModification_fromStr(_fact_d, ontology);
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + " & " + _fact_b, ontology),
                        _worldStateModification_fromStr(_fact_c, ontology));
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions), ontology);
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  assert_eq<std::string>(action1, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_true(_hasFact(problem.worldState, _fact_e, ontology));
}

void _checkSimpleExists()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location");
  ontology.constants = cp::SetOfEntities::fromStr("kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?l - location)\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(?l))", ontology),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkExistsWithActionParameterInvolved()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location");
  ontology.constants = cp::SetOfEntities::fromStr("pen - entity\n"
                                                  "kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?l - location, ?e - entity)\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> actionParameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(?l, ?obj))", ontology, actionParameters),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen, pen)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkExistsWithManyFactsInvolved()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "bottle mouse pen - entity\n"
                                                  "bedroom kitchen livingroom - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot, ?l - location)\n" +
                                                     _fact_b + "(?e - entity, ?l - location)\n" +
                                                     _fact_c, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> actionParameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, actionParameters),
                        _worldStateModification_fromStr(_fact_c, ontology, actionParameters));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?obj -> bottle)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _doAnActionToSatisfyAnExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "bottle mouse pen - entity\n"
                                                  "bedroom kitchen livingroom - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot, ?l - location)\n" +
                                                     _fact_b + "(?e - entity, ?l - location)\n" +
                                                     _fact_c + "(?e - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc - location", ontology)};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)", ontology)});

  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkForAllEffectAtStart()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "bottle mouse pen - entity\n"
                                                  "bedroom entrance kitchen livingroom - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot, ?l - location)\n" +
                                                     _fact_b + "(?e - entity, ?l - location)\n" +
                                                     _fact_c + "(?e - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc - location", ontology)};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.effect.worldStateModificationAtStart = _worldStateModification_fromStr("forall(?l - location, when(" + _fact_a + "(self, ?l), not(" + _fact_a + "(self, ?l))))", ontology, act2Parameters);
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)", ontology)});

  _addFact(problem.worldState, _fact_a + "(self, entrance)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_a + "(self, entrance)", ontology));
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_a + "(self, entrance)", ontology)); // removed by the effect at start of action2
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _existsWithValue()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "location\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "pen - entity\n"
                                                  "livingroom - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot) - location\n" +
                                                     _fact_b + "(?e - entity) - location\n" +
                                                     _fact_c + "(?e - entity)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj - entity", ontology)};
  cp::Action actionObj1(_condition_fromStr("exists(?l - location, " + _fact_a + "(self)=?l & " + _fact_b + "(?obj)=?l)", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc - location", ontology)};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self)=?loc", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(pen)", ontology)});

  _addFact(problem.worldState, _fact_b + "(pen)=livingroom", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action2 + "(?loc -> livingroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _notExists()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "kitchen - location", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot, ?l - location)\n" +
                                                     _fact_b, ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("not(exists(?l - location, " + _fact_a + "(self, ?l)))", ontology),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _actionToSatisfyANotExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("location\n"
                                           "robot\n"
                                           "resource");
  ontology.constants = cp::SetOfEntities::fromStr("self - robot\n"
                                                  "kitchen - location\n"
                                                  "spec_rec - resource", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?r - robot, ?l - location)\n" +
                                                     _fact_b + "\n" +
                                                     "busy(?r - resource)", ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("not(busy(spec_rec)) & not(exists(?l - location, " + _fact_a + "(self, ?l)))", ontology),
                        _worldStateModification_fromStr("not(busy(spec_rec)) & " + _fact_b, ontology));
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc - location", ontology)};
  cp::Action actionObj2({}, _worldStateModification_fromStr("!" + _fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  std::vector<cp::Parameter> act3Parameters{_parameter("?r - resource", ontology)};
  cp::Action actionObj3({}, _worldStateModification_fromStr("not(busy(?r))", ontology, act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  _addFact(problem.worldState, "busy(spec_rec)", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action3 + "(?r -> spec_rec)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action2 + "(?loc -> kitchen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _orInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "\n" +
                                                     _fact_b + "\n" +
                                                     _fact_c, ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr(_fact_a + " & " + _fact_b, ontology),
                        _worldStateModification_fromStr(_fact_c, ontology));
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + " | " + _fact_b, ontology),
                        _worldStateModification_fromStr(_fact_c, ontology));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + " & " + _fact_b, ontology),
                        _worldStateModification_fromStr(_fact_c, ontology));
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});
  _addFact(problem.worldState, _fact_a, problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _axioms()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("object");
  ontology.constants = cp::SetOfEntities::fromStr("book titi - object", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?o - object)\n" +
                                                     _fact_b + "(?o - object)\n" +
                                                     _fact_c + "(?o - object)\n" +
                                                     _fact_d + "(?o - object)\n" +
                                                     _fact_e, ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o - object", ontology)};
  cp::Axiom axiom(_condition_fromStr(_fact_a + "(?o)" + " & " + _fact_b + "(?o)", ontology, derPred1Parameters),
                  _fact(_fact_c + "(?o)", ontology, derPred1Parameters), derPred1Parameters);
  for (auto& currEvent : axiom.toEvents(ontology, {}))
    setOfEvents.add(currEvent);

  std::vector<cp::Parameter> derPred2Parameters{_parameter("?o - object", ontology)};
  cp::Axiom derivedPredicate2(_condition_fromStr(_fact_a + "(?o)" + " | " + _fact_b + "(?o)", ontology, derPred2Parameters),
                              _fact(_fact_d + "(?o)", ontology, derPred2Parameters), derPred2Parameters);
  for (auto& currEvent : derivedPredicate2.toEvents(ontology, {}))
    setOfEvents.add(currEvent);

  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack, ontology, setOfEventsMap, _now);

  assert_false(_hasFact(problem.worldState, _fact_c + "(book)", ontology));
  assert_false(_hasFact(problem.worldState, _fact_d + "(book)", ontology));
  _addFact(problem.worldState, _fact_a + "(book)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)", ontology));
  _addFact(problem.worldState, _fact_b + "(book)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(book)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)", ontology));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)", ontology));
  assert_false(_hasFact(problem.worldState, _fact_d + "(titi)", ontology));
  _removeFact(problem.worldState, _fact_a + "(book)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)", ontology));
  _addFact(problem.worldState, _fact_a + "(titi)", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)", ontology));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)", ontology));
  assert_true(_hasFact(problem.worldState, _fact_d + "(titi)", ontology));
}


void _assignAnotherValueToSatisfyNotGoal()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto titi - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + "=toto", ontology)));

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi", ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _assignUndefined()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity");
  ontology.constants = cp::SetOfEntities::fromStr("toto titi - entity", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("assign(" + _fact_a + ", undefined)", ontology)));

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi", ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq<std::size_t>(1u, problem.worldState.facts().size());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::size_t>(0u, problem.worldState.facts().size()); // because assign undefined is done like a fact removal
}


void _assignAFact()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal - entity\n"
                                                  "p1 p2 p3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=valGoal", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _assignAFactToAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal - entity\n"
                                                  "p1 p2 p3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "=valGoal", ontology),
                                      _worldStateModification_fromStr(_fact_c, ontology)));

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _assignAFactThenCheckEqualityWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc - param", ontology)};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr(_fact_d, ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _assignAFactThenCheckExistWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d, ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  cp::Action action2Obj(_condition_fromStr("exists(?pc - param, =(" + _fact_a + ", " + _fact_c + "(?pc)))", ontology),
                        _worldStateModification_fromStr(_fact_d, ontology));
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _existWithEqualityInEvent()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?p - param) - entity\n" +
                                                     _fact_b + "(?p - param)\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d + "(?p - param)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o - param", ontology)};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", ontology, action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o - param", ontology)};
  cp::Axiom derivedPredicate1(_condition_fromStr("exists(?pc - param, =(" + _fact_c + "(?pc), " + _fact_a + "(?o)))", ontology, derPred1Parameters),
                              _fact(_fact_b + "(?o)", ontology, derPred1Parameters), derPred1Parameters);
  for (auto& currEvent : derivedPredicate1.toEvents(ontology, {}))
    setOfEvents.add(currEvent);


  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)", ontology)});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)", ontology)});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
}


void _existWithEqualityInEvent_withEqualityInverted()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?p - param) - entity\n" +
                                                     _fact_b + "(?p - param)\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d + "(?p - param)", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o - param", ontology)};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", ontology, action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o - param", ontology)};
  cp::Axiom derivedPredicate1(_condition_fromStr("exists(?pc - param, =(" + _fact_a + "(?o), " + _fact_c + "(?pc)))", ontology, derPred1Parameters),
                              _fact(_fact_b + "(?o)", ontology, derPred1Parameters), derPred1Parameters);
  for (auto& currEvent : derivedPredicate1.toEvents(ontology, {}))
    setOfEvents.add(currEvent);


  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)", ontology)});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)", ontology)});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
}



void _fixEventWithFluentInParameter()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 titi - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?p - param) - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d + "\n" +
                                                     _fact_e + "(?p - param) - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + "(titi), " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc - param", ontology)};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_e + "(titi), " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr(_fact_d, ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?a - param", ontology), _parameter("?v - entity", ontology)};
  cp::Axiom derivedPredicate1(_condition_fromStr(_fact_a + "(?a)=?v", ontology, derPred1Parameters),
                              _fact(_fact_e + "(?a)=?v", ontology, derPred1Parameters), derPred1Parameters);
  for (auto& currEvent : derivedPredicate1.toEvents(ontology, {}))
    setOfEvents.add(currEvent);

  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  assert_eq<std::string>("action: action1\n"
                         "----------------------------------\n"
                         "\n"
                         "fact: fact_a(titi)=*\n"
                         "event: soe_from_constructor|event\n"
                         "\n"
                         "\n"
                         "event: soe_from_constructor|event\n"
                         "----------------------------------\n"
                         "\n"
                         "fact: fact_e(?a)=?v\n"
                         "action: action2\n", domain.printSuccessionCache());
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _derivedPredicates()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 titi - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + "(?p - param) - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d, ontology.types);
  ontology.derivedPredicates.addDerivedPredicate(
        cp::DerivedPredicate(cp::Predicate(_fact_e + "(?a - param) - entity", ontology.types),
                             _fact_a + "(?a)=?entity", ontology));

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + "(titi), " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc - param", ontology)};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_e + "(titi), " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr(_fact_d, ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::SetOfEvents setOfEvents;

  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));
  assert_eq<std::string>("action: action1\n"
                         "----------------------------------\n"
                         "\n"
                         "fact: fact_a(titi)=*\n"
                         "action: action2\n", domain.printSuccessionCache());

  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _assignAFactTwoTimesInTheSamePlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 kitchen - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d + "(?p - param) - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc - param", ontology)};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=kitchen", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _checkTwoTimesTheEqualityOfAFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("entity\n"
                                           "param");
  ontology.constants = cp::SetOfEntities::fromStr("aVal anotherVal valGoal v1 v2 v3 bedroom kitchen - entity\n"
                                                  "p1 p2 p3 pc1 pc2 pc3 - param", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr(_fact_a + " - entity\n" +
                                                     _fact_b + "(?p - param) - entity\n" +
                                                     _fact_c + "(?p - param) - entity\n" +
                                                     _fact_d + "(?p - param) - entity\n" +
                                                     _fact_e + "(?p - entity) - entity\n" +
                                                     _fact_f + " - entity", ontology.types);

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p - param", ontology)};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc - param", ontology)};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  std::vector<cp::Parameter> action3Parameters{_parameter("?tt - entity", ontology)};
  cp::Action action3Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_e + "(?tt))", ontology, action3Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_f + ", ?tt)", ontology, action3Parameters));
  action3Obj.parameters = std::move(action3Parameters);
  actions.emplace(action3, action3Obj);

  cp::Domain domain(std::move(actions), ontology);
  auto& setOfEventsMap = domain.getSetOfEvents();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_f + "=bedroom", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, _fact_e + "(bedroom)=kitchen", problem.goalStack, ontology, setOfEventsMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action3 + "(?tt -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _eventToRemoveAFactWithoutFluent()
{
  cp::Ontology ontology;
  ontology.types = cp::SetOfTypes::fromStr("loc_type\n"
                                           "robot");
  ontology.constants = cp::SetOfEntities::fromStr("me - robot\n"
                                                  "house1 house2 city anotherCity - loc_type", ontology.types);
  ontology.predicates = cp::SetOfPredicates::fromStr("locationOfRobot(?r - robot) - loc_type\n"
                                                     "robotAt(?r - robot, ?l - loc_type)\n"
                                                     "within(?l1 - loc_type) - loc_type", ontology.types);

  std::map<std::string, cp::Action> actions;
  cp::SetOfEvents setOfEvents;
  std::vector<cp::Parameter> inf1Parameters{_parameter("?l - loc_type", ontology)};
  cp::Event event1(_condition_fromStr("locationOfRobot(me)=?l", ontology, inf1Parameters),
                   _worldStateModification_fromStr("robotAt(me, ?l)", ontology, inf1Parameters));
  event1.parameters = std::move(inf1Parameters);
  setOfEvents.add(event1);

  std::vector<cp::Parameter> inf2Parameters{_parameter("?l - loc_type", ontology)};
  cp::Event event2(_condition_fromStr("exists(?loc - loc_type, locationOfRobot(me)=?loc & within(?loc)=?l)", ontology, inf2Parameters),
                   _worldStateModification_fromStr("robotAt(me, ?l)", ontology, inf2Parameters));
  event2.parameters = std::move(inf2Parameters);
  setOfEvents.add(event2);

  std::vector<cp::Parameter> inf3Parameters{_parameter("?l - loc_type", ontology)};
  cp::Event event3(_condition_fromStr("!locationOfRobot(me)=?l", ontology, inf3Parameters),
                   _worldStateModification_fromStr("forall(?ll - loc_type, robotAt(me, ?ll), !robotAt(me, ?ll))", ontology, inf3Parameters));
  event3.parameters = std::move(inf3Parameters);
  setOfEvents.add(event3);
  cp::Domain domain(std::move(actions), ontology, std::move(setOfEvents));

  auto& setOfEventsMap = domain.getSetOfEvents();

  cp::Problem problem;
  _addFact(problem.worldState, "within(house1)=city", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "within(house2)=city", problem.goalStack, ontology, setOfEventsMap, _now);
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)", ontology));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)", ontology));
  _removeFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)", ontology));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)", ontology));
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)", ontology));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)", ontology));
  _addFact(problem.worldState, "locationOfRobot(me)=city", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)", ontology));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)", ontology));
  _addFact(problem.worldState, "locationOfRobot(me)=anotherCity", problem.goalStack, ontology, setOfEventsMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)", ontology));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)", ontology));
}




}




void test_planWithSingleType()
{
  _test_createEmptyGoal();
  _test_goalToStr();
  _test_factToStr();
  _test_conditionParameters();
  _test_wsModificationToStr();
  _test_invertCondition();
  _test_checkCondition();
  _automaticallyRemoveGoalsWithAMaxTimeToKeepInactiveEqualTo0();
  _maxTimeToKeepInactiveEqualTo0UnderAnAlreadySatisfiedGoal();
  _noPreconditionGoalImmediatlyReached();
  _removeGoalWhenItIsSatisfiedByAnAction();
  _removeAnAction();
  _removeSomeGoals();
  _notifyGoalRemovedWhenItIsImmediatlyRemoved();
  _handlePreconditionWithNegatedFacts();
  _testWithNegatedAccessibleFacts();
  _noPlanWithALengthOf2();
  _noPlanWithALengthOf3();
  _2preconditions();
  _2Goals();
  _2UnrelatedGoals();
  _impossibleGoal();
  _privigelizeTheActionsThatHaveManyPreferedInContext();
  _preconditionThatCannotBeSolved();
  _preferInContext();
  _preferWhenPreconditionAreCloserToTheRealFacts();
  _avoidToDo2TimesTheSameActionIfPossble();
  _takeHistoricalIntoAccount();
  _goDoTheActionThatHaveTheMostPreferInContextValidated();
  _checkNotInAPrecondition();
  _checkClearGoalsWhenItsAlreadySatisfied();
  _checkActionHasAFact();
  _precoditionEqualEffect();
  _addGoalEvenForEmptyAction();
  _circularDependencies();
  _triggerActionThatRemoveAFact();
  _actionWithConstantValue();
  _actionWithParameterizedValue();
  _actionWithParameterizedParameter();
  _actionWithParametersInPreconditionsAndEffectsWithoutSolution();
  _actionWithParametersInsideThePath();
  _testPersistGoal();
  _testPersistImplyGoal();
  _testImplyGoal();
  _checkPreviousBugAboutSelectingAnInappropriateAction();
  _dontLinkActionWithPreferredInContext();
  _checkPriorities();
  _stackablePropertyOfGoals();
  _doNotRemoveAGoalWithMaxTimeToKeepInactiveEqual0BelowAGoalWithACondotionNotSatisfied();
  _checkMaxTimeToKeepInactiveForGoals();
  _changePriorityOfGoal();
  _factChangedNotification();
  _checkEvents();
  _checkEventsWithImply();
  _checkEventWithPunctualCondition();
  _checkEventAtEndOfAPlan();
  _checkEventInsideAPlan();
  _checkEventThatAddAGoal();
  _testGetNotSatisfiedGoals();
  _testGoalUnderPersist();
  _checkLinkedEvents();
  _oneStepTowards();
  _infrenceLinksFromManyEventsSets();
  _factValueModification();
  _removeGoaWhenAnActionFinishesByAddingNewGoals();
  _setWsModification();
  _forAllWsModification();
  _actionNavigationAndGrabObjectWithParameters();
  _actionNavigationAndGrabObjectWithParameters2();
  _moveObject();
  _moveAndUngrabObject();
  _failToMoveAnUnknownObject();
  _completeMovingObjectScenario();
  _eventWithANegatedFactWithParameter();
  _actionWithANegatedFactNotTriggeredIfNotNecessary();
  _useTwoTimesAnEvent();
  _linkWithAnyValueInCondition();
  _removeAFactWithAnyValue();
  _notDeducePathIfTheParametersOfAFactAreDifferents();
  _checkPreferInContext();
  _checkPreferHighImportanceOfNotRepeatingIt();
  _actionWithFactWithANegatedFact();
  _negatedFactValueInWorldState();
  _problemThatUseADomainThatChangedSinceLastUsage();
  _checkFilterFactInCondition();
  _checkFilterFactInConditionAndThenPropagate();
  _checkOutputValueOfLookForAnActionToDo();
  _hardProblemThatNeedsToBeSmart();
  _goalsToDoInParallel();
  _checkOverallEffectDuringParallelisation();
  _checkSimpleExists();
  _checkExistsWithActionParameterInvolved();
  _checkExistsWithManyFactsInvolved();
  _doAnActionToSatisfyAnExists();
  _checkForAllEffectAtStart();
  _existsWithValue();
  _notExists();
  _actionToSatisfyANotExists();
  _orInCondition();
  _axioms();
  _assignAnotherValueToSatisfyNotGoal();
  _assignUndefined();
  _assignAFact();
  _assignAFactToAction();
  _assignAFactThenCheckEqualityWithAnotherFact();
  _assignAFactThenCheckExistWithAnotherFact();
  _existWithEqualityInEvent();
  _existWithEqualityInEvent_withEqualityInverted();
  _fixEventWithFluentInParameter();
  _derivedPredicates();
  _assignAFactTwoTimesInTheSamePlan();
  _checkTwoTimesTheEqualityOfAFact();
  _eventToRemoveAFactWithoutFluent();

  std::cout << "chatbot planner without types is ok !!!!" << std::endl;
}
