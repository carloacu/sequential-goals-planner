#include "test_plannerWithSingleType.hpp"
#include <iostream>
#include <assert.h>
#include <contextualplanner/contextualplanner.hpp>
#include <contextualplanner/types/derivedpredicate.hpp>
#include <contextualplanner/types/predicate.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/trackers/goalsremovedtracker.hpp>
#include <contextualplanner/util/print.hpp>

namespace
{
const std::map<cp::SetOfInferencesId, cp::SetOfInferences> _emptySetOfInferences;
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
               const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::Fact(pStr, {}, {}, pParameters);
}

cp::Parameter _parameter(const std::string& pStr) {
  return cp::Parameter(pStr, {});
}

cp::Entity _entity(const std::string& pStr) {
  return cp::Entity(pStr, {});
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
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  std::vector<cp::Goal> goals;
  for (auto& currFactStr : pGoalStrs)
    goals.emplace_back(currFactStr, cp::Ontology(), cp::SetOfEntities());
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
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  std::map<int, std::vector<cp::Goal>> goals;
  for (auto& currGoals : pGoalStrs)
  {
    auto& goalsForAPrio = goals[currGoals.first];
    for (auto& currGoal : currGoals.second)
      goalsForAPrio.emplace_back(currGoal, cp::Ontology(), cp::SetOfEntities());
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
               const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  std::map<int, std::vector<cp::Goal>> goals;
  for (auto& currGoals : pGoalStrs)
  {
    auto& goalsForAPrio = goals[currGoals.first];
    for (auto& currGoal : currGoals.second)
      goalsForAPrio.emplace_back(currGoal, cp::Ontology(), cp::SetOfEntities());
  }
  pProblem.goalStack.addGoals(goals, pProblem.worldState, pNow);
}

void _addGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<std::string>& pGoalStrs,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::GoalStack::defaultPriority)
{
  std::vector<cp::Goal> goals;
  for (auto& currFactStr : pGoalStrs)
    goals.emplace_back(currFactStr, cp::Ontology(), cp::SetOfEntities());
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
               const std::map<cp::SetOfInferencesId, cp::SetOfInferences>& pSetOfInferences = _emptySetOfInferences) {
  std::set<cp::Fact> facts;
  for (auto& currFactStr : pFactStrs)
    facts.emplace(currFactStr, cp::Ontology(), cp::SetOfEntities(), std::vector<cp::Parameter>());
  pWorldState.setFacts(facts, pGoalStack, pSetOfInferences, cp::Ontology(), cp::SetOfEntities(), _now);
}

void _addFact(cp::WorldState& pWorldState,
              const std::string& pFactStr,
              cp::GoalStack& pGoalStack,
              const std::map<cp::SetOfInferencesId, cp::SetOfInferences>& pSetOfInferences = _emptySetOfInferences,
              const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {}) {
  pWorldState.addFact(_fact(pFactStr), pGoalStack, pSetOfInferences, cp::Ontology(), cp::SetOfEntities(), pNow);
}

void _removeFact(cp::WorldState& pWorldState,
                 const std::string& pFactStr,
                 cp::GoalStack& pGoalStack,
                 const std::map<cp::SetOfInferencesId, cp::SetOfInferences>& pSetOfInferences = _emptySetOfInferences,
                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {}) {
  pWorldState.removeFact(_fact(pFactStr), pGoalStack, pSetOfInferences, cp::Ontology(), cp::SetOfEntities(), pNow);
}

bool _hasFact(cp::WorldState& pWorldState,
              const std::string& pFactStr) {
  return pWorldState.hasFact(_fact(pFactStr));
}


std::string _solveStrConst(const cp::Problem& pProblem,
                           const std::map<std::string, cp::Action>& pActions,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  cp::Domain domain(pActions);
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
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions);
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
  _goal("goal_name", ontology, -1, "");
}

void _test_goalToStr()
{
  cp::Ontology ontology;
  assert_eq<std::string>("persist(a & b)", _goal("persist(a & b)", ontology).toStr());
  assert_eq<std::string>("imply(condition, goal_name)", _goal("imply(condition, goal_name)", ontology).toStr());
  assert_eq<std::string>("persist(imply(condition, goal_name))", _goal("persist(imply(condition, goal_name))", ontology).toStr());
  assert_eq<std::string>("oneStepTowards(goal_name)", _goal("oneStepTowards(goal_name)", ontology).toStr());
}

void _test_factToStr()
{
  assert_eq<std::string>("isEngaged(1)", _fact("isEngaged(1)").toStr());
}




void _test_conditionParameters()
{
  cp::Ontology ontology;
  assert_false(_condition_fromStr("", ontology).operator bool());

  std::vector<cp::Parameter> parameters = {_parameter("?target"), _parameter("?object")};
  std::map<cp::Parameter, cp::Entity> parametersToEntities = {{_parameter("?target"), _entity("kitchen")}, {_parameter("?object"), _entity("chair")}};
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)", _condition_fromStr("location(me)=?target & grab(me, ?object)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)", _condition_fromStr("and(location(me)=?target, grab(me, ?object))", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair) & i", _condition_fromStr("and(location(me)=?target, grab(me, ?object), i)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | i", _condition_fromStr("location(me)=?target | grab(me, ?object) | i", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | i", _condition_fromStr("or(location(me)=?target, grab(me, ?object), i)", ontology, parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("equals(a, b + 3)", _condition_fromStr("equals(a, b + 3)", ontology)->toStr());
  assert_eq<std::string>("!a", _condition_fromStr("!a", ontology)->toStr());
  assert_eq<std::string>("!a", _condition_fromStr("not(a)", ontology)->toStr());
  assert_eq<std::string>("a!=", _condition_fromStr("a!=", ontology)->toStr());
  assert_eq<std::string>("a!=b", _condition_fromStr("a!=b", ontology)->toStr());
  assert_eq<std::string>("a>3", _condition_fromStr("a>3", ontology)->toStr());
  assert_eq<std::string>("a>3", _condition_fromStr(">(a, 3)", ontology)->toStr());
  assert_eq<std::string>("a<3", _condition_fromStr("<(a, 3)", ontology)->toStr());
  assert_eq<std::string>("!a=*", _condition_fromStr("=(a, undefined)", ontology)->toStr());
  assert_eq<std::string>("!a(b)=*", _condition_fromStr("=(a(b), undefined)", ontology)->toStr());
  assert_eq<std::string>("!a=*", _condition_fromStr("a=undefined", ontology)->toStr());
  assert_eq<std::string>("!a(b)=*", _condition_fromStr("a(b)=undefined", ontology)->toStr());
  assert_eq<std::string>("a=c", _condition_fromStr("=(a, c)", ontology)->toStr());
  assert_eq<std::string>("a(b)=c", _condition_fromStr("=(a(b), c)", ontology)->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("=(a, c())", ontology)->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("=(a(b), c())", ontology)->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("equals(a, c)", ontology)->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("equals(a(b), c)", ontology)->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("equals(a, c())", ontology)->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("equals(a(b), c())", ontology)->toStr());
  assert_eq<std::string>("exists(l, at(self, l))", _condition_fromStr("exists(l, at(self, l))", ontology)->toStr());
  assert_eq<std::string>("exists(l, at(self, l) & at(pen, l))", _condition_fromStr("exists(l, at(self, l) & at(pen, l))", ontology)->toStr());
  assert_eq<std::string>("!exists(l, at(self, l))", _condition_fromStr("not(exists(l, at(self, l)))", ontology)->toStr());
  assert_eq<std::string>("!(equals(at(i, o), at(self, l)))", _condition_fromStr("not(equals(at(i, o), at(self, l)))", ontology)->toStr());
  assert_eq<std::string>("!(equals(at(i, o), at(self, l)))", _condition_fromStr("not(=(at(i, o), at(self, l)))", ontology)->toStr());
}

void _test_wsModificationToStr()
{
  cp::Ontology ontology;
  assert_false(_worldStateModification_fromStr("", ontology).operator bool());
  assert_eq<std::string>("location(me)=target", _worldStateModification_fromStr("location(me)=target", ontology)->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("location(me)=target & grab(sweets)", ontology)->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("and(location(me)=target, grab(sweets))", ontology)->toStr());
  assert_eq<std::string>("assign(a, b + 3)", _worldStateModification_fromStr("assign(a, b + 3)", ontology)->toStr());
  assert_eq<std::string>("assign(a, b + 4 - 1)", _worldStateModification_fromStr("set(a, b + 4 - 1)", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("increase(a, 1)", _worldStateModification_fromStr("add(a, 1)", ontology)->toStr());
  assert_eq<std::string>("increase(a, 1)", _worldStateModification_fromStr("increase(a, 1)", ontology)->toStr());
  assert_eq<std::string>("decrease(a, 2)", _worldStateModification_fromStr("decrease(a, 2)", ontology)->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("!a", ontology)->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("not(a)", ontology)->toStr());
  assert_eq<std::string>("!a=*", _worldStateModification_fromStr("a=undefined", ontology)->toStr());
  assert_eq<std::string>("!a=*", _worldStateModification_fromStr("assign(a, undefined)", ontology)->toStr());
  assert_eq<std::string>("forall(a, f(a), d(a, c))", _worldStateModification_fromStr("forall(a, f(a), d(a, c))", ontology)->toStr());
  assert_eq<std::string>("forall(a, f(a), d(a, c))", _worldStateModification_fromStr("forall(a, when(f(a), d(a, c)))", ontology)->toStr());
  assert_eq<std::string>("forall(a, f(a), !d(a, c))", _worldStateModification_fromStr("forall(a, when(f(a), not(d(a, c))))", ontology)->toStr());
  assert_eq<std::string>("assign(a(b), c(d))", _worldStateModification_fromStr("assign(a(b), c(d))", ontology)->toStr());
  assert_eq<std::string>("assign(a(b), c(d))", _worldStateModification_fromStr("set(a(b), c(d))", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("assign(a(b), c())", ontology)->toStr()); // c() means that c is a predicate
  assert_eq<std::string>("a(b)=c", _worldStateModification_fromStr("assign(a(b), c)", ontology)->toStr());
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("set(a(b), c())", ontology)->toStr()); // set is depecated
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("set(a(b), c)", ontology)->toStr()); // set is depecated
}

void _test_invertCondition()
{
  cp::Ontology ontology;
  assert_eq<std::string>("!location(me)=kitchen | grab(me, chair) | !i", _condition_fromStr("location(me)=kitchen & !grab(me, chair) & i)", ontology)->clone(nullptr, true)->toStr());
  assert_eq<std::string>("!location(me)=kitchen & !grab(me, chair) & i", _condition_fromStr("location(me)=kitchen | grab(me, chair) | !i", ontology)->clone(nullptr, true)->toStr());
  assert_eq<std::string>("equals(at(i, o), at(self, l))", _condition_fromStr("not(=(at(i, o), at(self, l)))", ontology)->clone(nullptr, true)->toStr());
}


void _test_checkCondition()
{
  cp::Ontology ontology;
  cp::WorldState worldState;
  cp::GoalStack goalStack;
  std::map<cp::SetOfInferencesId, cp::SetOfInferences> setOfInferences;
  worldState.addFact(_fact("a=c"), goalStack, setOfInferences, {}, {}, {});
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_false(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  assert_eq<std::size_t>(1, worldState.facts().size());
  worldState.addFact(_fact("a!=c"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_false(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a!=b"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(2, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a=d"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c", ontology)->isTrue(worldState));
  assert_true(_condition_fromStr("a=d", ontology)->isTrue(worldState));
  assert_false(_condition_fromStr("a!=d", ontology)->isTrue(worldState));
  worldState.addFact(_fact("a!=c"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a=d", ontology)->isTrue(worldState));
}


void _automaticallyRemoveGoalsWithAMaxTimeToKeepInactiveEqualTo0()
{
  cp::Ontology ontology;
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
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"});
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain));
  assert_true(!problem.goalStack.goals().empty());
  assert_true(!_hasFact(problem.worldState, "be_happy"));
  _addFact(problem.worldState, "be_happy", problem.goalStack);
  assert_true(_hasFact(problem.worldState, "be_happy"));
}


void _removeGoalWhenItIsSatisfiedByAnAction()
{
  cp::Ontology ontology;
  ontology.predicates = cp::SetOfPredicates::fromStr("be_happy", ontology.types);

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr("be_happy", ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"});

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
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"});
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
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {"be_happy"});
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
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_checkedIn});
  auto actionToDo = _lookForAnActionToDoStr(problem, domain);
  assert_true(actionToDo == _action_greet || actionToDo == _action_joke);
}


void _testWithNegatedAccessibleFacts()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_e + " & !" + _fact_b, ontology),
                                      _worldStateModification_fromStr("!" + _fact_c, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b, ontology)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_a + " & !" + _fact_c, ontology),
                                      _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_a, _fact_b, _fact_c}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_d});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
  _addFact(problem.worldState, _fact_e,  problem.goalStack);
  assert_eq(action2, _lookForAnActionToDoStr(problem, domain));
}

void _noPlanWithALengthOf2()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _noPlanWithALengthOf3()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2preconditions()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2Goals()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2UnrelatedGoals()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _impossibleGoal()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _privigelizeTheActionsThatHaveManyPreferedInContext()
{
  cp::Ontology ontology;

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
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}

void _preconditionThatCannotBeSolved()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkInWithQrCode, cp::Action(_condition_fromStr(_fact_hasQrCode, ontology),
                                                        _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_checkInWithPassword, cp::Action(_condition_fromStr(_fact_hasCheckInPasword, ontology),
                                                          _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _preferInContext()
{
  cp::Ontology ontology;

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
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack);
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  _setFacts(problem.worldState, {}, problem.goalStack);
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasQrCode}, problem.goalStack);
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_hasCheckInPasword}, problem.goalStack);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  actions.emplace(_action_checkInWithRealPerson, cp::Action({},
                                                            _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                                            _condition_fromStr("!" + _fact_hasQrCode, ontology)));
  _setFacts(problem.worldState, {}, problem.goalStack);
  assert_eq(_action_checkInWithRealPerson + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology),
                                            _condition_fromStr(_fact_beginOfConversation, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setFacts(problem.worldState, {_fact_beginOfConversation}, problem.goalStack);
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _avoidToDo2TimesTheSameActionIfPossble()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology)));
  actions.emplace(_action_presentation, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _solveStr(problem, actions));

  _setFacts(problem.worldState, {}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _takeHistoricalIntoAccount()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));

  assert_eq(_action_presentation + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));
}


void _goDoTheActionThatHaveTheMostPreferInContextValidated()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, _worldStateModification_fromStr(_fact_advertised, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_is_close, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology),
                                              _condition_fromStr(_fact_is_close, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_advertised + "&" + _fact_checkedIn, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_is_close}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoStr(problem, domain));
}


void _checkNotInAPrecondition()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(_condition_fromStr("!" + _fact_checkedIn, ontology),
                                            _worldStateModification_fromStr(_fact_greeted, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  problem.worldState.modify(_worldStateModification_fromStr(_fact_checkedIn, ontology), problem.goalStack,
                            _emptySetOfInferences, {}, {}, _now);
  assert_eq(std::string(), _lookForAnActionToDoConstStr(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  cp::Ontology ontology;

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_greeted}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  cp::Domain domain;
  _lookForAnActionToDo(problem, domain);
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());
}


void _checkActionHasAFact()
{
  cp::Ontology ontology;

  cp::ProblemModification effect(_worldStateModification_fromStr(_fact_a + " & !" + _fact_b, ontology));
  effect.potentialWorldStateModification = _worldStateModification_fromStr(_fact_c, ontology);
  effect.goalsToAdd[cp::GoalStack::defaultPriority] = {_goal(_fact_d, ontology)};
  const cp::Action action(_condition_fromStr(_fact_e, ontology),
                          effect,
                          _condition_fromStr(_fact_f, ontology));
  assert_true(action.hasFact(_fact(_fact_a)));
  assert_true(action.hasFact(_fact(_fact_b)));
  assert_true(action.hasFact(_fact(_fact_c)));
  assert_true(action.hasFact(_fact(_fact_d)));
  assert_true(action.hasFact(_fact(_fact_e)));
  assert_true(action.hasFact(_fact(_fact_f)));
  assert_false(action.hasFact(_fact(_fact_g)));
}


void _precoditionEqualEffect()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_beHappy, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _addGoalEvenForEmptyAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  cp::Action act1Obj({}, {});
  act1Obj.effect.goalsToAddInCurrentPriority.push_back(_goal(_fact_a, ontology));
  actions.emplace(action1, act1Obj);
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  assert_true(problem.goalStack.goals().empty());
  cp::notifyActionDone(problem, domain, cp::ActionInvocationWithGoal(action1, {}, {}, 0), {});
  assert_false(problem.goalStack.goals().empty());
}

void _circularDependencies()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace("check-in-pwd", cp::Action(_condition_fromStr(_fact_hasCheckInPasword, ontology),
                                             _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace("inverse-of-check-in-pwd", cp::Action(_condition_fromStr(_fact_checkedIn, ontology),
                                                        _worldStateModification_fromStr(_fact_hasCheckInPasword, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action(_condition_fromStr(_fact_beSad, ontology),
                                           _worldStateModification_fromStr("!" + _fact_beSad, ontology)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr("!" + _fact_beSad, ontology),
                                              _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Historical historical;
  cp::Problem problem;
  _addFact(problem.worldState, _fact_beSad, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStr(problem, actions, {}, &historical));
}


void _actionWithConstantValue()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, _worldStateModification_fromStr("place=kitchen", ontology));
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen", ontology)});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _actionWithParameterizedValue()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?target"));
  cp::Action navigate({}, _worldStateModification_fromStr("place=?target", ontology, parameters));
  navigate.parameters = std::move(parameters);
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen", ontology)});
  assert_eq(_action_navigate + "(?target -> kitchen)", _solveStr(problem, actions));
}


void _actionWithParameterizedParameter()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human"));
  cp::Action joke({}, _worldStateModification_fromStr("isHappy(?human)", ontology, parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("isHappy(1)", ontology)});
  assert_eq(_action_joke + "(?human -> 1)", _solveStr(problem, actions));
}


void _actionWithParametersInPreconditionsAndEffectsWithoutSolution()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human"));
  cp::Action joke(_condition_fromStr("isEngaged(?human)", ontology, parameters),
                  _worldStateModification_fromStr("isHappy(?human)", ontology, parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _addFact(problem.worldState, "isEngaged(2)", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("isHappy(1)", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions));
}

void _actionWithParametersInsideThePath()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navigateParameters(1, _parameter("?target"));
  cp::Action navigateAction({},
                            _worldStateModification_fromStr("place=?target", ontology, navigateParameters));
  navigateAction.parameters = std::move(navigateParameters);
  actions.emplace(_action_navigate, navigateAction);

  actions.emplace(_action_welcome,
                  cp::Action(_condition_fromStr("place=entrance", ontology),
                             _worldStateModification_fromStr("welcomePeople", ontology)));

  cp::Problem problem;
  _addFact(problem.worldState, "place=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("welcomePeople", ontology)});
  assert_eq<std::string>(_action_navigate + "(?target -> entrance)" + _sep +
                         _action_welcome, _solveStr(problem, actions));
  assert_true(_hasFact(problem.worldState, "place=entrance"));
  assert_false(_hasFact(problem.worldState, "place=kitchen"));
}


void _testPersistGoal()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_welcome, cp::Action({}, _worldStateModification_fromStr("welcomePeople", ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("welcomePeople", ontology)});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());

  problem = cp::Problem();
  _setGoalsForAPriority(problem, {_goal("persist(welcomePeople)", ontology)});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
}


void _testPersistImplyGoal()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_greeted + ", " + _fact_checkedIn + "))", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions));
  _addFact(problem.worldState, _fact_greeted, problem.goalStack);
  assert_eq<std::string>(_action_checkIn, _solveStr(problem, actions));
}


void _testImplyGoal()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_greeted + ", " + _fact_checkedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, actions));
  // It is not a persistent goal it is removed
  _addFact(problem.worldState, _fact_greeted, problem.goalStack);
  assert_eq<std::string>("", _solveStr(problem, actions));
}


void _checkPreviousBugAboutSelectingAnInappropriateAction()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action(_condition_fromStr(_fact_engagedWithUser, ontology),
                                                   _worldStateModification_fromStr(_fact_userSatisfied, ontology),
                                                   _condition_fromStr("!" + _fact_robotLearntABehavior, ontology)));
  actions.emplace(_action_checkIn, cp::Action({},
                                              _worldStateModification_fromStr("!" + _fact_robotLearntABehavior + " & " + _fact_advertised, ontology)));
  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack);
  _setGoalsForAPriority(problem, {"persist(" + _fact_userSatisfied + ")"});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
  _removeFact(problem.worldState, _fact_userSatisfied, problem.goalStack);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _dontLinkActionWithPreferredInContext()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_userSatisfied, ontology),
                                                   _condition_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_engagedWithUser, ontology),
                                              _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Historical historical;
  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_userSatisfied});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _checkPriorities()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_fact_greeted}}, {9, {_fact_beHappy}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _stackablePropertyOfGoals()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted, ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted, ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  problem2.goalStack.pushFrontGoal(_goal(_fact_presented, ontology), problem2.worldState, {}, 10);
  assert_eq(_action_presentation + _sep +
            _action_goodBoy, _solveStr(problem2, actions));
}



void _doNotRemoveAGoalWithMaxTimeToKeepInactiveEqual0BelowAGoalWithACondotionNotSatisfied()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented, ontology)));

  // Even if _fact_checkedIn has maxTimeToKeepInactive equal to 0, it is not removed because the goal with a higher priority is inactive.
  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  _addFact(problem2.worldState, _fact_presented, problem2.goalStack); // The difference here is that the condition of the first goal is satisfied
  _setGoals(problem2, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem2, actions));


  cp::Problem problem3;
  _setGoals(problem3, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  _addFact(problem3.worldState, _fact_presented, problem3.goalStack); // The difference here is that the condition is validated after the add of the goal
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem3, actions));


  cp::Problem problem4;
  _setGoals(problem4, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", ontology, 0)}}, {9, {_goal(_fact_checkedIn, ontology, 0), _goal(_fact_beHappy, ontology)}}});
  _addFact(problem4.worldState, _fact_presented, problem4.goalStack); // Here _fact_checkedIn goal shoud be removed from the stack
  _removeFact(problem4.worldState, _fact_presented, problem4.goalStack); // The difference here is that the condition was validated only punctually
  assert_eq(_action_goodBoy, _solveStr(problem4, actions));
}



void _checkMaxTimeToKeepInactiveForGoals()
{
  cp::Ontology ontology;

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));


  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted, ontology), _goal(_fact_checkedIn, ontology, 60)}}}, now);
  assert_eq(_action_greet + _sep +
            _action_checkIn, _solveStr(problem, actions, now));


  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted, ontology), _goal(_fact_checkedIn, ontology, 60)}}}, now);
  now = std::make_unique<std::chrono::steady_clock::time_point>(*now + std::chrono::seconds(100));
  assert_eq(_action_greet, _solveStr(problem2, actions, now));
}



void _changePriorityOfGoal()
{
  cp::Ontology ontology;

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

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}});
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

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}});
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

  _setGoals(problem, {{10, {_fact_greeted, _fact_checkedIn}}});
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

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted, ontology)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn + "&" + _fact_punctual_p1, ontology)));
  cp::Domain domain(std::move(actions));

  std::set<cp::Fact> factsChangedFromSubscription;
  cp::Problem problem;
  _addFact(problem.worldState, _fact_beginOfConversation, problem.goalStack);
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

  _setGoals(problem, {{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}});
  assert_eq({}, factsChangedFromSubscription);

  auto plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_greet, plannerResult.actionInvocation.actionId);
  assert_eq({_fact(_fact_beginOfConversation), _fact(_fact_greeted)}, factsChangedFromSubscription);
  assert_eq({}, punctualFactsAdded);
  assert_eq({_fact(_fact_greeted)}, factsAdded);
  factsAdded.clear();
  assert_eq({}, factsRemoved);

  plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_checkIn, plannerResult.actionInvocation.actionId);
  assert_eq({_fact(_fact_beginOfConversation), _fact(_fact_greeted), _fact(_fact_checkedIn)}, factsChangedFromSubscription);
  assert_eq({_fact(_fact_punctual_p1)}, punctualFactsAdded);
  assert_eq({_fact(_fact_checkedIn)}, factsAdded);
  assert_eq({}, factsRemoved);
  _removeFact(problem.worldState, _fact_greeted, problem.goalStack);
  assert_eq({_fact(_fact_beginOfConversation), _fact(_fact_checkedIn)}, factsChangedFromSubscription);
  assert_eq({_fact(_fact_punctual_p1)}, punctualFactsAdded);
  assert_eq({_fact(_fact_checkedIn)}, factsAdded);
  assert_eq({_fact(_fact_greeted)}, factsRemoved);

  onFactsRemovedConnection.disconnect();
  onPunctualFactsConnection.disconnect();
  onFactsAddedConnection.disconnect();
  factsChangedConnection.disconnect();
}




void _checkInferences()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  // Inference: if (_fact_headTouched) then remove(_fact_headTouched) and addGoal(_fact_checkedIn)
  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_headTouched, ontology),
                                          _worldStateModification_fromStr("!" + _fact_headTouched, ontology),
                                          _emptyParameters, {{{9, {_goal(_fact_checkedIn, ontology)}}}}));
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, setOfInferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}



void _checkInferencesWithImply()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn, ontology)));

  // Inference: if (_fact_headTouched) then add(_fact_userWantsToCheckedIn) and remove(_fact_headTouched)
  cp::Domain domain(std::move(actions), {},
                    cp::Inference(_condition_fromStr(_fact_headTouched, ontology),
                                  _worldStateModification_fromStr(_fact_userWantsToCheckedIn + " & !" + _fact_headTouched, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_userWantsToCheckedIn + ", " + _fact_checkedIn + "))", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkInferenceWithPunctualCondition()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn, ontology)));

  // Inference: if (_fact_punctual_headTouched) then add(_fact_userWantsToCheckedIn)
  cp::Domain domain(std::move(actions), {},
                    cp::Inference(_condition_fromStr(_fact_punctual_headTouched, ontology),
                                  _worldStateModification_fromStr(_fact_userWantsToCheckedIn, ontology)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkInferenceAtEndOfAPlan()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_punctual_checkedIn, ontology)));

  cp::SetOfInferences setOfInferences;
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_headTouched, ontology),
                                             _worldStateModification_fromStr(_fact_userWantsToCheckedIn, ontology)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_checkedIn, ontology),
                                             _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn, ontology)));
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  assert_eq<std::string>("", _solveStr(problem, domain, _now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, setOfReferencesMap, _now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, _now));
}


void _checkInferenceInsideAPlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_c, ontology), _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions));

  {
    cp::SetOfInferences setOfInferences;
    setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_a, ontology),
                                               _worldStateModification_fromStr(_fact_b, ontology)));
    setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_b + "&" + _fact_d, ontology),
                                               _worldStateModification_fromStr(_fact_c, ontology)));
    domain.addSetOfInferences(std::move(setOfInferences));
  }

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  assert_eq<std::string>("", _solveStrConst(problem, domain));

  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_b, ontology),
                                          _worldStateModification_fromStr(_fact_c, ontology)));

  assert_eq(action1 + _sep + action2, _solveStrConst(problem, domain)); // check with a copy of the problem
  assert_true(!_hasFact(problem.worldState, _fact_a));
  assert_true(!_hasFact(problem.worldState, _fact_b));
  assert_true(!_hasFact(problem.worldState, _fact_c));
  assert_true(!_hasFact(problem.worldState, _fact_d));
  assert_eq(action1 + _sep + action2, _solveStr(problem, domain));
  assert_true(_hasFact(problem.worldState, _fact_a));
  assert_true(_hasFact(problem.worldState, _fact_b));
  assert_true(_hasFact(problem.worldState, _fact_c));
  assert_true(_hasFact(problem.worldState, _fact_d));
}


void _checkInferenceThatAddAGoal()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  const std::string action4 = "action4";
  const std::string action5 = "action5";

  cp::Ontology ontology;

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
  cp::SetOfInferences setOfInferences;
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_a, ontology),
                                             _worldStateModification_fromStr(_fact_b, ontology),
                                             _emptyParameters, {{cp::GoalStack::defaultPriority, {_goal(_fact_e, ontology)}}}));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_b, ontology),
                                             _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_g + ", " + _fact_d + ")", ontology)});
  assert_eq<std::string>("", _solveStrConst(problem, domain));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_g, problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + _sep + action4 + _sep + action3 + _sep + action2, _solveStr(problem, domain));
}


void _testGetNotSatisfiedGoals()
{
  auto goal1 = "persist(!" + _fact_a + ")";
  auto goal2 = "persist(" + _fact_b + ")";
  auto goal3 = "imply(" + _fact_c + ", " + _fact_d + ")";
  auto goal4 = "persist(imply(!" + _fact_c + ", " + _fact_d + "))";

  cp::Ontology ontology;

  cp::Problem problem;
  _addGoalsForAPriority(problem, {goal1}, {}, cp::GoalStack::defaultPriority + 1);
  _addGoalsForAPriority(problem, {goal2, goal3, goal4});

  assert_eq(goal1 + ", " + goal2 + ", " + goal3 + ", " + goal4, cp::printGoals(problem.goalStack.goals()));
  assert_eq(goal2 + ", " + goal4, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_a, problem.goalStack);
  assert_eq(goal1 + ", " + goal2 + ", " + goal4, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_c, problem.goalStack);
  assert_eq(goal1 + ", " + goal2 + ", " + goal3, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_d, problem.goalStack);
  assert_eq(goal1 + ", " + goal2, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _removeFact(problem.worldState, _fact_a, problem.goalStack);
  assert_eq(goal2, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _addFact(problem.worldState, _fact_b, problem.goalStack);
  assert_eq<std::string>("", cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
  _removeFact(problem.worldState, _fact_d, problem.goalStack);
  assert_eq(goal3, cp::printGoals(problem.goalStack.getNotSatisfiedGoals(problem.worldState)));
}



void _testGoalUnderPersist()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions));

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    _addFact(problem.worldState, _fact_a, problem.goalStack);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    _addGoalsForAPriority(problem, {_goal(_fact_b, ontology, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {_fact_c}, now, cp::GoalStack::defaultPriority + 2);
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

    _removeFact(problem.worldState, _fact_c, problem.goalStack);
    problem.goalStack.pushBackGoal(_goal(_fact_c, ontology), problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    auto plannerResult = _lookForAnActionToDo(problem, domain, now);
    assert_eq(action2, plannerResult.actionInvocation.actionId);

    now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now() + std::chrono::minutes(5));
    cp::notifyActionDone(problem, domain, plannerResult, now);

    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, now);
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId); // Not action1 because it was inactive for too long
  }

}


void _checkLinkedInferences()
{
  cp::Ontology ontology;

  cp::SetOfInferences setOfInferences;
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")", ontology)});
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p2, ontology),
                                             _worldStateModification_fromStr(_fact_a, ontology)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p5, ontology),
                                             _worldStateModification_fromStr(_fact_punctual_p2 + "&" + _fact_punctual_p3, ontology)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p4, ontology),
                                             _worldStateModification_fromStr(_fact_punctual_p5 + "&" + _fact_punctual_p1, ontology)));

  std::map<cp::SetOfInferencesId, cp::SetOfInferences> setOfInferencesMap = {{"soi", setOfInferences}};
  assert_false(_hasFact(problem.worldState, _fact_a));
  _addFact(problem.worldState, _fact_punctual_p4, problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_a));
}



void _oneStepTowards()
{
  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification greetPbModification;
  greetPbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_greeted, ontology);
  actions.emplace(_action_greet, cp::Action({}, greetPbModification));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy, ontology)));
  static const std::string actionb = "actionb";
  actions.emplace(actionb, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions));

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
  _addFact(problem.worldState, _fact_a, problem.goalStack);
  assert_eq(actionb, _lookForAnActionToDoStr(problem, domain, _now));
  assert_eq<std::string>(actionb + _sep + _action_goodBoy, _solveStrConst(problem, domain));
  assert_eq<std::string>(implyGoal.toStr() + _sep + _fact_beHappy, _getGoalsDoneDuringAPlannificationConst(problem, domain));
}


void _infrenceLinksFromManyInferencesSets()
{
  cp::Ontology ontology;

  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification pbModification;
  pbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_d, ontology);
  actions.emplace(action1, cp::Action({}, pbModification));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c, ontology)));
  cp::Domain domain(std::move(actions));

  assert_true(cp::GoalStack::defaultPriority >= 1);
  auto lowPriority = cp::GoalStack::defaultPriority - 1;
  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_punctual_p2, ontology),
                                          {}, _emptyParameters,
                                          {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")", ontology)}}}));
  cp::Problem problem;
  _setGoals(problem, {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")", ontology, 0)}}});

  {
    cp::SetOfInferences setOfInferences2;
    setOfInferences2.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p1, ontology),
                                                _worldStateModification_fromStr(_fact_b + "&" + _fact_punctual_p2, ontology)));
    setOfInferences2.addInference(cp::Inference(_condition_fromStr(_fact_b, ontology),
                                                {}, _emptyParameters,
                                                {{cp::GoalStack::defaultPriority, {_goal("oneStepTowards(" + _fact_c + ")", ontology)}}}));
    domain.addSetOfInferences(setOfInferences2);
  }

  auto& setOfInferencesMap = domain.getSetOfInferences();
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  _addFact(problem.worldState, _fact_punctual_p1, problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
}


void _factValueModification()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("persist(imply(" + _fact_a + "=a, " + "!" + _fact_b + "))", ontology, 0)}}});

  _addFact(problem.worldState, _fact_b, problem.goalStack);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, _now));
  _addFact(problem.worldState, _fact_a + "=a", problem.goalStack);
  assert_eq(action1, _lookForAnActionToDoStr(problem, domain, _now));
  _addFact(problem.worldState, _fact_a + "=b", problem.goalStack);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, _now));
}


void _removeGoaWhenAnActionFinishesByAddingNewGoals()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification wm(_worldStateModification_fromStr(_fact_a, ontology));
  wm.goalsToAddInCurrentPriority.push_back(_goal(_fact_b, ontology, 0));
  actions.emplace(action1, cp::Action({}, wm));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  _setGoals(problem, {{27, {_fact_a}}}, _now);

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

  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("set(location(me), location(object))", ontology));
  actions.emplace(_action_navigate, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(object)=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("location(me)=kitchen", ontology)});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _forAllWsModification()
{
  cp::Ontology ontology;

  const std::string action1 = "action1";
  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("forAll(?obj, grab(me, ?obj), set(location(?obj), location(me)))", ontology));
  actions.emplace(action1, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(object1)=kitchen", problem.goalStack);
  _addFact(problem.worldState, "grab(me, object1)", problem.goalStack);
  _addFact(problem.worldState, "grab(me, object2)", problem.goalStack);

  _setGoalsForAPriority(problem, {_goal("location(object2)=corridor", ontology)});
  assert_eq(action1, _solveStr(problem, actions));
}


void _actionNavigationAndGrabObjectWithParameters()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)"))->toStr());
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)", _solveStr(problem, actions));
}


void _actionNavigationAndGrabObjectWithParameters2()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(?object), location(me))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)"))->toStr());
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)", _solveStr(problem, actions));
}



void _moveObject()
{
  const std::string actionNavigate2 = "actionNavigate2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> nav2Parameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Action navAction2(_condition_fromStr("grab(me, ?object)", ontology, nav2Parameters),
                        _worldStateModification_fromStr("location(me)=?targetLocation & location(?object)=?targetLocation", ontology, nav2Parameters));
  navAction2.parameters = std::move(nav2Parameters);
  actions.emplace(actionNavigate2, navAction2);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("location(sweets)=bedroom", ontology)});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets), " +
                         actionNavigate2 + "(?object -> sweets, ?targetLocation -> bedroom)", _solveStr(problem, actions));
}


void _moveAndUngrabObject()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters(1, _parameter("?object"));
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::Problem problem;
  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", ontology, inferenceParameters),
                          _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", ontology, inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, setOfInferencesMap, _now);
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)", ontology)});

  assert_eq(_action_navigate + "(?targetLocation -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "locationOfObj(sweets)=kitchen"));
}


void _failToMoveAnUnknownObject()
{
  const std::string actionWhereIsObject = "actionWhereIsObject";
  const std::string actionLeavePod = "actionLeavePod";
  const std::string actionRanomWalk = "actionRanomWalk";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction(_condition_fromStr("!isLost & !charging(me)", ontology, navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  cp::Action randomWalkAction(_condition_fromStr("!charging(me)", ontology),
                       _worldStateModification_fromStr("!isLost", ontology));
  actions.emplace(actionRanomWalk, randomWalkAction);

  cp::Action leavePodAction({}, _worldStateModification_fromStr("!charging(me)", ontology));
  actions.emplace(actionLeavePod, leavePodAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object"), _parameter("?aLocation")};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObj(?object)=*", ontology, whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObj(?object)=?aLocation", ontology, whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object")};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object")};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", ontology, inferenceParameters),
                          _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", ontology, inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)", ontology)});

  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "charging(me)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, setOfInferencesMap, _now);
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

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters{_parameter("?targetPlace")};
  cp::Action navAction(_condition_fromStr("!lost(me) & !pathIsBlocked", ontology, navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetPlace", ontology, navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object")};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObject(?object)) & not(exists(o, grab(me)=o))", ontology, grabParameters),
                        _worldStateModification_fromStr("grab(me)=?object", ontology, grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object")};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me)=?object", ontology, ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object"), _parameter("?aLocation")};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObject(?object)=*", ontology, whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObject(?object)=?aLocation", ontology, whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object"), _parameter("?location")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?location & grab(me)=?object", ontology, inferenceParameters),
                          _worldStateModification_fromStr("locationOfObject(?object)=?location", ontology, inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets", ontology)});

  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "locationOfObject(sweets)=kitchen", problem.goalStack, setOfInferencesMap, _now);
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets", ontology)});
  assert_eq(_action_navigate + "(?targetPlace -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetPlace -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal("locationOfObject(bottle)=entrance & !grab(me)=bottle", ontology)});
  assert_eq(actionWhereIsObject + "(?aLocation -> entrance, ?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}

void _inferenceWithANegatedFactWithParameter()
{
  const std::string actionUngrabLeftHand = "actionUngrabLeftHand";
  const std::string actionUngrabRightHand = "actionUngrabRightHand";
  const std::string actionUngrabBothHands = "actionUngrabBothHands";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> ungrabLeftParameters{_parameter("?object")};
  cp::Action ungrabLeftAction(_condition_fromStr("!hasTwoHandles(?object)", ontology, ungrabLeftParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object", ontology, ungrabLeftParameters));
  ungrabLeftAction.parameters = std::move(ungrabLeftParameters);
  actions.emplace(actionUngrabLeftHand, ungrabLeftAction);

  std::vector<cp::Parameter> ungrabRightParameters{_parameter("?object")};
  cp::Action ungrabRightAction(_condition_fromStr("!hasTwoHandles(?object)", ontology, ungrabRightParameters),
                               _worldStateModification_fromStr("!grabRightHand(me)=?object", ontology, ungrabRightParameters));
  ungrabRightAction.parameters = std::move(ungrabRightParameters);
  actions.emplace(actionUngrabRightHand, ungrabRightAction);

  std::vector<cp::Parameter> ungrabBothParameters{_parameter("?object")};
  cp::Action ungrabBothAction(_condition_fromStr("hasTwoHandles(?object)", ontology, ungrabBothParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", ontology, ungrabBothParameters));
  ungrabBothAction.parameters = std::move(ungrabBothParameters);
  actions.emplace(actionUngrabBothHands, ungrabBothAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object")};
  cp::Inference inference(_condition_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", ontology, inferenceParameters),
                          _worldStateModification_fromStr("!grab(me, ?object)", ontology, inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, "hasTwoHandles(sweets)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grabLeftHand(me)=sweets", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grabRightHand(me)=sweets", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grab(me, sweets)", problem.goalStack, setOfInferencesMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, sweets)", ontology)});

  assert_eq(actionUngrabBothHands + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, sweets)"));

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, setOfInferencesMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, bottle)", ontology)});
  assert_eq(actionUngrabLeftHand + "(?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grabRightHand(me)=glass", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "grab(me, glass)", problem.goalStack, setOfInferencesMap, _now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, glass)", ontology)});
  assert_eq(actionUngrabRightHand + "(?object -> glass)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, glass)"));
}


void _actionWithANegatedFactNotTriggeredIfNotNecessary()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr("!" + _fact_a, ontology),
                                      _condition_fromStr(_fact_c + " & " + _fact_d, ontology)));

  actions.emplace(action2, cp::Action(_condition_fromStr("!" + _fact_a + " & " + _fact_e, ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_e, ontology)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_c, problem.goalStack);
  _addFact(problem.worldState, _fact_d, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _useTwoTimesAnInference()
{
  cp::Ontology ontology;

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object")};
  cp::Inference inference(_condition_fromStr(_fact_a + " & " + _fact_b + "(?object)", ontology, inferenceParameters),
                          _worldStateModification_fromStr(_fact_c + "(?object)", ontology, inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);

  std::map<std::string, cp::Action> actions;
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj2)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj1)"));
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj2)"));
}


void _linkWithAnyValueInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr("!" + _fact_a + "=*", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));

  std::vector<cp::Parameter> act2Parameters{_parameter("?aVal")};
  cp::Action act2({}, _worldStateModification_fromStr("!" + _fact_a + "=?aVal", ontology, act2Parameters));
  act2.parameters = std::move(act2Parameters);
  actions.emplace(action2, act2);

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "=toto", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  assert_eq(action2 + "(?aVal -> toto)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _removeAFactWithAnyValue()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + " & !" + _fact_b + "=*", ontology)));
  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "=toto", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_a, ontology)});
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_b + "=toto"));
}



void _notDeducePathIfTheParametersOfAFactAreDifferents()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "(1)", ontology),
                                      _condition_fromStr(_fact_c, ontology)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "(2)", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_b, ontology),
                                      _worldStateModification_fromStr(_fact_d, ontology)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "(2)", problem.goalStack);
  _addFact(problem.worldState, _fact_c, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.actionId);
}


void _checkPreferInContext()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology), _condition_fromStr(_fact_b, ontology)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_a});
  assert_eq(action1, _solveStrConst(problem, actions, &problem.historical));
  assert_eq(action1, _solveStrConst(problem, actions, &problem.historical));
}


void _checkPreferHighImportanceOfNotRepeatingIt()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  auto action1Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology), _condition_fromStr(_fact_b, ontology));
  action1Obj.highImportanceOfNotRepeatingIt = true;
  actions.emplace(action1, action1Obj);
  auto action2Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology));
  action2Obj.highImportanceOfNotRepeatingIt = true;
  actions.emplace(action2, action2Obj);

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_a});
  assert_eq(action1, _solveStrConst(problem, actions, &problem.historical));
  assert_eq(action2, _solveStrConst(problem, actions, &problem.historical));
}


void _actionWithFactWithANegatedFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "=a", ontology)));

  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "!=b & " + _fact_d, ontology),
                                      _worldStateModification_fromStr(_fact_c, ontology)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_d, ontology),
                                      _condition_fromStr(_fact_e, ontology)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _negatedFactValueInWorldState()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a + "!=b", ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions));

  {
    cp::Problem problem;
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=b", problem.goalStack);
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=c", problem.goalStack);
    _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

    assert_eq<std::string>(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "!=b", problem.goalStack);
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

  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a, ontology),
                                      _worldStateModification_fromStr(_fact_b, ontology)));
  cp::Domain domain(std::move(actions));

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

  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj"), _parameter("?loc")};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "(?obj)", ontology, act2Parameters),
                        _worldStateModification_fromStr(_fact_c, ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);
  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj1, loc2)", problem.goalStack, setOfInferencesMap, _now);
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

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj"), _parameter("?loc")};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "=?obj", ontology, act2Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);


  std::vector<cp::Parameter> act3Parameters{_parameter("?loc")};
  cp::Action actionObj3(_condition_fromStr(_fact_c + "(?loc)", ontology, act3Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?loc)", ontology, act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(obj2, loc2)", problem.goalStack, setOfInferencesMap, _now);
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

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a, ontology)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
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
    _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, _now);
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

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
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

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, setOfInferencesMap, _now);
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

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
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

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, setOfInferencesMap, _now);
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

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  assert_eq<std::string>(action1, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, _now));
  assert_true(_hasFact(problem.worldState, _fact_e));
}

void _checkSimpleExists()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(?l))", ontology),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkExistsWithActionParameterInvolved()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> actionParameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(?l, ?obj))", ontology, actionParameters),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen, pen)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkExistsWithManyFactsInvolved()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> actionParameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, actionParameters),
                        _worldStateModification_fromStr(_fact_c, ontology, actionParameters));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});

  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?obj -> bottle)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _doAnActionToSatisfyAnExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)", ontology)});

  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _checkForAllEffectAtStart()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.effect.worldStateModificationAtStart = _worldStateModification_fromStr("forall(?l, when(" + _fact_a + "(self, ?l), not(" + _fact_a + "(self, ?l))))", ontology, act2Parameters);
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)", ontology)});

  _addFact(problem.worldState, _fact_a + "(self, entrance)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_a + "(self, entrance)"));
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_a + "(self, entrance)")); // removed by the effect at start of action2
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _existsWithValue()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self)=?l & " + _fact_b + "(?obj)=?l)", ontology, act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", ontology, act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self)=?loc", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(pen)", ontology)});

  _addFact(problem.worldState, _fact_b + "(pen)=livingroom", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action2 + "(?loc -> livingroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _notExists()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("not(exists(?l, " + _fact_a + "(self, ?l)))", ontology),
                        _worldStateModification_fromStr(_fact_b, ontology));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});

  assert_eq(action1, _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _actionToSatisfyANotExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::Action actionObj1(_condition_fromStr("not(busy(spec_rec)) & not(exists(?l, " + _fact_a + "(self, ?l)))", ontology),
                        _worldStateModification_fromStr("not(busy(spec_rec)) & " + _fact_b, ontology));
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr("!" + _fact_a + "(self, ?loc)", ontology, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  std::vector<cp::Parameter> act3Parameters{_parameter("?r")};
  cp::Action actionObj3({}, _worldStateModification_fromStr("not(busy(?r))", ontology, act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b, ontology)});
  _addFact(problem.worldState, "busy(spec_rec)", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action3 + "(?r -> spec_rec)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action2 + "(?loc -> kitchen)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _orInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  cp::Ontology ontology;

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

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});
  _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _derivedPredicate()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr(_fact_a + "(?o)" + " & " + _fact_b + "(?o)", ontology, derPred1Parameters),
                                         _fact(_fact_c + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  std::vector<cp::Parameter> derPred2Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate2(_condition_fromStr(_fact_a + "(?o)" + " | " + _fact_b + "(?o)", ontology, derPred2Parameters),
                                         _fact(_fact_d + "(?o)", derPred2Parameters), derPred2Parameters);
  for (auto& currInference : derivedPredicate2.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack, setOfInferencesMap, _now);

  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_a + "(book)", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_b + "(book)", problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)"));
  assert_false(_hasFact(problem.worldState, _fact_d + "(titi)"));
  _removeFact(problem.worldState, _fact_a + "(book)", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_a + "(titi)", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(titi)"));
}


void _assignAnotherValueToSatisfyNotGoal()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + "=toto", ontology)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi", ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _assignUndefined()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("assign(" + _fact_a + ", undefined)", ontology)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi", ontology)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, setOfInferencesMap, _now);
  assert_eq<std::size_t>(1u, problem.worldState.facts().size());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::size_t>(0u, problem.worldState.facts().size()); // because assign undefined is done like a fact removal
}


void _assignAFact()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=valGoal", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _assignAFactToAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "=valGoal", ontology),
                                      _worldStateModification_fromStr(_fact_c, ontology)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}


void _assignAFactThenCheckEqualityWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr(_fact_d, ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _assignAFactThenCheckExistWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  cp::Action action2Obj(_condition_fromStr("exists(?pc, =(" + _fact_a + ", " + _fact_c + "(?pc)))", ontology),
                        _worldStateModification_fromStr(_fact_d, ontology));
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}


void _existWithEqualityInInference()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o")};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", ontology, action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr("exists(?pc, =(" + _fact_c + "(?pc), " + _fact_a + "(?o)))", ontology, derPred1Parameters),
                                         _fact(_fact_b + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);


  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)", ontology)});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)", ontology)});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
}


void _existWithEqualityInInference_withEqualityInverted()
{
  const std::string action1 = "action1";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o")};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", ontology, action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr("exists(?pc, =(" + _fact_a + "(?o), " + _fact_c + "(?pc)))", ontology, derPred1Parameters),
                                         _fact(_fact_b + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);


  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)", ontology)});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)", ontology)});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, _now).actionInvocation.toStr());
}



void _fixInferenceWithFluentInParameter()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + "(titi), " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_e + "(titi), " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr(_fact_d, ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?a"), _parameter("?v")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr(_fact_a + "(?a)=?v", ontology, derPred1Parameters),
                                         _fact(_fact_e + "(?a)=?v", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d, ontology)});
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, _now).actionInvocation.toStr());
}



void _assignAFactTwoTimesInTheSamePlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=kitchen", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, setOfInferencesMap, _now);

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

  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", ontology, action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", ontology, action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", ontology, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  std::vector<cp::Parameter> action3Parameters{_parameter("?tt")};
  cp::Action action3Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_e + "(?tt))", ontology, action3Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_f + ", ?tt)", ontology, action3Parameters));
  action3Obj.parameters = std::move(action3Parameters);
  actions.emplace(action3, action3Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_f + "=bedroom", ontology)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, _fact_e + "(bedroom)=kitchen", problem.goalStack, setOfInferencesMap, _now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq(action3 + "(?tt -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, _now).actionInvocation.toStr());
}



void _inferenceToRemoveAFactWithoutFluent()
{
  cp::Ontology ontology;

  std::map<std::string, cp::Action> actions;
  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inf1Parameters{_parameter("?l")};
  cp::Inference inference1(_condition_fromStr("locationOfRobot(me)=?l", ontology, inf1Parameters),
                          _worldStateModification_fromStr("robotAt(me, ?l)", ontology, inf1Parameters));
  inference1.parameters = std::move(inf1Parameters);
  setOfInferences.addInference(inference1);

  std::vector<cp::Parameter> inf2Parameters{_parameter("?l")};
  cp::Inference inference2(_condition_fromStr("exists(?loc, locationOfRobot(me)=?loc & within(?loc)=?l)", ontology, inf2Parameters),
                          _worldStateModification_fromStr("robotAt(me, ?l)", ontology, inf2Parameters));
  inference2.parameters = std::move(inf2Parameters);
  setOfInferences.addInference(inference2);

  std::vector<cp::Parameter> inf3Parameters{_parameter("?l")};
  cp::Inference inference3(_condition_fromStr("!locationOfRobot(me)=?l", ontology, inf3Parameters),
                          _worldStateModification_fromStr("forall(?ll, robotAt(me, ?ll), !robotAt(me, ?ll))", ontology, inf3Parameters));
  inference3.parameters = std::move(inf3Parameters);
  setOfInferences.addInference(inference3);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, "within(house1)=city", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "within(house2)=city", problem.goalStack, setOfInferencesMap, _now);
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _removeFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, _now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=city", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=anotherCity", problem.goalStack, setOfInferencesMap, _now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)"));
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
  _checkInferences();
  _checkInferencesWithImply();
  _checkInferenceWithPunctualCondition();
  _checkInferenceAtEndOfAPlan();
  _checkInferenceInsideAPlan();
  _checkInferenceThatAddAGoal();
  _testGetNotSatisfiedGoals();
  _testGoalUnderPersist();
  _checkLinkedInferences();
  _oneStepTowards();
  _infrenceLinksFromManyInferencesSets();
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
  _inferenceWithANegatedFactWithParameter();
  _actionWithANegatedFactNotTriggeredIfNotNecessary();
  _useTwoTimesAnInference();
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
  _derivedPredicate();
  _assignAnotherValueToSatisfyNotGoal();
  _assignUndefined();
  _assignAFact();
  _assignAFactToAction();
  _assignAFactThenCheckEqualityWithAnotherFact();
  _assignAFactThenCheckExistWithAnotherFact();
  _existWithEqualityInInference();
  _existWithEqualityInInference_withEqualityInverted();
  _fixInferenceWithFluentInParameter();
  _assignAFactTwoTimesInTheSamePlan();
  _checkTwoTimesTheEqualityOfAFact();
  _inferenceToRemoveAFactWithoutFluent();

  std::cout << "chatbot planner without types is ok !!!!" << std::endl;
}
