#include "test_plannerWithoutTypes.hpp"
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
               int pMaxTimeToKeepInactive = -1,
               const std::string& pGoalGroupId = "") {
  return cp::Goal(pStr, {}, {}, pMaxTimeToKeepInactive, pGoalGroupId);
}

std::unique_ptr<cp::Condition> _condition_fromStr(const std::string& pConditionStr,
                                                  const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::Condition::fromStr(pConditionStr, {}, {}, pParameters);
}

std::unique_ptr<cp::WorldStateModification> _worldStateModification_fromStr(const std::string& pStr,
                                                                            const std::vector<cp::Parameter>& pParameters = {}) {
  return cp::WorldStateModification::fromStr(pStr, {}, {}, pParameters);
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
  _goal("goal_name", -1, "");
}

void _test_goalToStr()
{
  assert_eq<std::string>("persist(a & b)", _goal("persist(a & b)").toStr());
  assert_eq<std::string>("imply(condition, goal_name)", _goal("imply(condition, goal_name)").toStr());
  assert_eq<std::string>("persist(imply(condition, goal_name))", _goal("persist(imply(condition, goal_name))").toStr());
  assert_eq<std::string>("oneStepTowards(goal_name)", _goal("oneStepTowards(goal_name)").toStr());
}

void _test_factToStr()
{
  assert_eq<std::string>("isEngaged(1)", _fact("isEngaged(1)").toStr());
}




void _test_conditionParameters()
{
  assert_false(_condition_fromStr("").operator bool());

  std::vector<cp::Parameter> parameters = {_parameter("?target"), _parameter("?object")};
  std::map<cp::Parameter, cp::Entity> parametersToEntities = {{_parameter("?target"), _entity("kitchen")}, {_parameter("?object"), _entity("chair")}};
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)", _condition_fromStr("location(me)=?target & grab(me, ?object)", parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair)", _condition_fromStr("and(location(me)=?target, grab(me, ?object))", parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen & grab(me, chair) & i", _condition_fromStr("and(location(me)=?target, grab(me, ?object), i)", parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | i", _condition_fromStr("location(me)=?target | grab(me, ?object) | i", parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("location(me)=kitchen | grab(me, chair) | i", _condition_fromStr("or(location(me)=?target, grab(me, ?object), i)", parameters)->clone(&parametersToEntities)->toStr());
  assert_eq<std::string>("equals(a, b + 3)", _condition_fromStr("equals(a, b + 3)")->toStr());
  assert_eq<std::string>("!a", _condition_fromStr("!a")->toStr());
  assert_eq<std::string>("!a", _condition_fromStr("not(a)")->toStr());
  assert_eq<std::string>("a!=", _condition_fromStr("a!=")->toStr());
  assert_eq<std::string>("a!=b", _condition_fromStr("a!=b")->toStr());
  assert_eq<std::string>("a>3", _condition_fromStr("a>3")->toStr());
  assert_eq<std::string>("a>3", _condition_fromStr(">(a, 3)")->toStr());
  assert_eq<std::string>("a<3", _condition_fromStr("<(a, 3)")->toStr());
  assert_eq<std::string>("!a=*", _condition_fromStr("=(a, undefined)")->toStr());
  assert_eq<std::string>("!a(b)=*", _condition_fromStr("=(a(b), undefined)")->toStr());
  assert_eq<std::string>("!a=*", _condition_fromStr("a=undefined")->toStr());
  assert_eq<std::string>("!a(b)=*", _condition_fromStr("a(b)=undefined")->toStr());
  assert_eq<std::string>("a=c", _condition_fromStr("=(a, c)")->toStr());
  assert_eq<std::string>("a(b)=c", _condition_fromStr("=(a(b), c)")->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("=(a, c())")->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("=(a(b), c())")->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("equals(a, c)")->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("equals(a(b), c)")->toStr());
  assert_eq<std::string>("equals(a, c)", _condition_fromStr("equals(a, c())")->toStr());
  assert_eq<std::string>("equals(a(b), c)", _condition_fromStr("equals(a(b), c())")->toStr());
  assert_eq<std::string>("exists(l, at(self, l))", _condition_fromStr("exists(l, at(self, l))")->toStr());
  assert_eq<std::string>("exists(l, at(self, l) & at(pen, l))", _condition_fromStr("exists(l, at(self, l) & at(pen, l))")->toStr());
  assert_eq<std::string>("!exists(l, at(self, l))", _condition_fromStr("not(exists(l, at(self, l)))")->toStr());
  assert_eq<std::string>("!(equals(at(i, o), at(self, l)))", _condition_fromStr("not(equals(at(i, o), at(self, l)))")->toStr());
  assert_eq<std::string>("!(equals(at(i, o), at(self, l)))", _condition_fromStr("not(=(at(i, o), at(self, l)))")->toStr());
}

void _test_wsModificationToStr()
{
  assert_false(_worldStateModification_fromStr("").operator bool());
  assert_eq<std::string>("location(me)=target", _worldStateModification_fromStr("location(me)=target")->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("location(me)=target & grab(sweets)")->toStr());
  assert_eq<std::string>("location(me)=target & grab(sweets)", _worldStateModification_fromStr("and(location(me)=target, grab(sweets))")->toStr());
  assert_eq<std::string>("assign(a, b + 3)", _worldStateModification_fromStr("assign(a, b + 3)")->toStr());
  assert_eq<std::string>("assign(a, b + 4 - 1)", _worldStateModification_fromStr("set(a, b + 4 - 1)")->toStr()); // set is depecated
  assert_eq<std::string>("increase(a, 1)", _worldStateModification_fromStr("add(a, 1)")->toStr());
  assert_eq<std::string>("increase(a, 1)", _worldStateModification_fromStr("increase(a, 1)")->toStr());
  assert_eq<std::string>("decrease(a, 2)", _worldStateModification_fromStr("decrease(a, 2)")->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("!a")->toStr());
  assert_eq<std::string>("!a", _worldStateModification_fromStr("not(a)")->toStr());
  assert_eq<std::string>("!a=*", _worldStateModification_fromStr("a=undefined")->toStr());
  assert_eq<std::string>("!a=*", _worldStateModification_fromStr("assign(a, undefined)")->toStr());
  assert_eq<std::string>("forall(a, f(a), d(a, c))", _worldStateModification_fromStr("forall(a, f(a), d(a, c))")->toStr());
  assert_eq<std::string>("forall(a, f(a), d(a, c))", _worldStateModification_fromStr("forall(a, when(f(a), d(a, c)))")->toStr());
  assert_eq<std::string>("forall(a, f(a), !d(a, c))", _worldStateModification_fromStr("forall(a, when(f(a), not(d(a, c))))")->toStr());
  assert_eq<std::string>("assign(a(b), c(d))", _worldStateModification_fromStr("assign(a(b), c(d))")->toStr());
  assert_eq<std::string>("assign(a(b), c(d))", _worldStateModification_fromStr("set(a(b), c(d))")->toStr()); // set is depecated
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("assign(a(b), c())")->toStr()); // c() means that c is a predicate
  assert_eq<std::string>("a(b)=c", _worldStateModification_fromStr("assign(a(b), c)")->toStr());
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("set(a(b), c())")->toStr()); // set is depecated
  assert_eq<std::string>("assign(a(b), c())", _worldStateModification_fromStr("set(a(b), c)")->toStr()); // set is depecated
}

void _test_invertCondition()
{
  assert_eq<std::string>("!location(me)=kitchen | grab(me, chair) | !i", _condition_fromStr("location(me)=kitchen & !grab(me, chair) & i)")->clone(nullptr, true)->toStr());
  assert_eq<std::string>("!location(me)=kitchen & !grab(me, chair) & i", _condition_fromStr("location(me)=kitchen | grab(me, chair) | !i")->clone(nullptr, true)->toStr());
  assert_eq<std::string>("equals(at(i, o), at(self, l))", _condition_fromStr("not(=(at(i, o), at(self, l)))")->clone(nullptr, true)->toStr());
}


void _test_checkCondition()
{
  cp::WorldState worldState;
  cp::GoalStack goalStack;
  std::map<cp::SetOfInferencesId, cp::SetOfInferences> setOfInferences;
  worldState.addFact(_fact("a=c"), goalStack, setOfInferences, {}, {}, {});
  assert_true(_condition_fromStr("a!=b")->isTrue(worldState));
  assert_false(_condition_fromStr("a!=c")->isTrue(worldState));
  assert_eq<std::size_t>(1, worldState.facts().size());
  worldState.addFact(_fact("a!=c"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_false(_condition_fromStr("a!=b")->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c")->isTrue(worldState));
  worldState.addFact(_fact("a!=b"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(2, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b")->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c")->isTrue(worldState));
  worldState.addFact(_fact("a=d"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a!=b")->isTrue(worldState));
  assert_true(_condition_fromStr("a!=c")->isTrue(worldState));
  assert_true(_condition_fromStr("a=d")->isTrue(worldState));
  assert_false(_condition_fromStr("a!=d")->isTrue(worldState));
  worldState.addFact(_fact("a!=c"), goalStack, setOfInferences, {}, {}, {});
  assert_eq<std::size_t>(1, worldState.facts().size());
  assert_true(_condition_fromStr("a=d")->isTrue(worldState));
}


void _automaticallyRemoveGoalsWithAMaxTimeToKeepInactiveEqualTo0()
{
  cp::GoalStack goalStack;
  cp::WorldState worldState;
  assert_eq<std::size_t>(0u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_advertised), worldState, {}, 10);
  goalStack.pushBackGoal(_goal(_fact_beHappy), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_checkedIn, 0), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
  assert_eq<std::size_t>(1u, goalStack.goals().find(9)->second.size());
}


void _maxTimeToKeepInactiveEqualTo0UnderAnAlreadySatisfiedGoal()
{
  cp::GoalStack goalStack;
  cp::WorldState worldState;
  assert_eq<std::size_t>(0u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal("persist(!" + _fact_a + ")"), worldState, {}, 10);
  assert_eq<std::size_t>(1u, goalStack.goals().size());
  goalStack.pushBackGoal(_goal(_fact_checkedIn, 0), worldState, {}, 9);
  assert_eq<std::size_t>(2u, goalStack.goals().size());
}

void _noPreconditionGoalImmediatlyReached()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain));
  assert_true(!problem.goalStack.goals().empty());
  assert_true(!_hasFact(problem.worldState, _fact_beHappy));
  _addFact(problem.worldState, _fact_beHappy, problem.goalStack);
  assert_true(_hasFact(problem.worldState, _fact_beHappy));
}


void _removeGoalWhenItIsSatisfiedByAnAction()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});

  auto plannerResult = _lookForAnActionToDoThenNotify(problem, domain);
  assert_eq(_action_goodBoy, plannerResult.actionInvocation.toStr());
  assert_eq(_fact_beHappy, plannerResult.fromGoal->toStr());
  assert_eq(10, plannerResult.fromGoalPriority);
  assert_true(problem.goalStack.goals().empty());
}


void _removeAnAction()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({},
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  domain.removeAction(_action_goodBoy);
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _removeSomeGoals()
{
  const std::string goalGroupId = "greetAndCheckIn";
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  problem.goalStack.pushFrontGoal(_goal(_fact_checkedIn, -1, goalGroupId), problem.worldState, {});
  problem.goalStack.pushFrontGoal(_goal(_fact_greeted, -1, goalGroupId), problem.worldState, {});
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
  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });

  problem.goalStack.addGoals({{10, {_goal(_fact_a)}}}, problem.worldState, {});

  problem.goalStack.addGoals({{9, {_goal(_fact_b, 0)}}}, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushBackGoal(_goal(_fact_c, 0), problem.worldState, {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_c, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushFrontGoal(_goal(_fact_d, 0), problem.worldState, {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_d, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.pushFrontGoal(_goal(_fact_e, 0), problem.worldState, {}, 11);
  assert_eq<std::size_t>(0u, goalsRemoved.size());
  problem.goalStack.changeGoalPriority(_fact_e, 9, true, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_e, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.goalStack.setGoals({{10, {_goal(_fact_a, 0)}}, {9, {_goal(_fact_b, 0)}}}, problem.worldState, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
  onGoalsRemovedConnection.disconnect();
}



void _handlePreconditionWithNegatedFacts()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(_condition_fromStr("!" + _fact_checkedIn),
                                            _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_joke, cp::Action(_condition_fromStr("!" + _fact_checkedIn),
                                           _worldStateModification_fromStr(_fact_userSatisfied)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_userSatisfied),
                                              _worldStateModification_fromStr(_fact_checkedIn)));
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
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_e + " & !" + _fact_b),
                                      _worldStateModification_fromStr("!" + _fact_c)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_a + " & !" + _fact_c),
                                      _worldStateModification_fromStr(_fact_d)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted),
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _noPlanWithALengthOf3()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted),
                                              _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _privigelizeTheActionsThatHaveManyPreferedInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_checkInWithQrCode, cp::Action(_condition_fromStr(_fact_hasQrCode),
                                                        _worldStateModification_fromStr(_fact_checkedIn),
                                                        _condition_fromStr(_fact_hasQrCode)));
  actions.emplace(_action_checkInWithPassword, cp::Action(_condition_fromStr(_fact_hasCheckInPasword),
                                                          _worldStateModification_fromStr(_fact_checkedIn),
                                                          _condition_fromStr(_fact_hasCheckInPasword)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkInWithQrCode, cp::Action(_condition_fromStr(_fact_hasQrCode),
                                                        _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_checkInWithPassword, cp::Action(_condition_fromStr(_fact_hasCheckInPasword),
                                                          _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _preferInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkInWithQrCode, cp::Action({},
                                                        _worldStateModification_fromStr(_fact_checkedIn),
                                                        _condition_fromStr(_fact_hasQrCode)));
  actions.emplace(_action_checkInWithPassword, cp::Action({},
                                                          _worldStateModification_fromStr(_fact_checkedIn),
                                                          _condition_fromStr(_fact_hasCheckInPasword)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_greeted + " & " + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));

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

  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
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
                                                            _worldStateModification_fromStr(_fact_checkedIn),
                                                            _condition_fromStr("!" + _fact_hasQrCode)));
  _setFacts(problem.worldState, {}, problem.goalStack);
  assert_eq(_action_checkInWithRealPerson + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented),
                                            _condition_fromStr(_fact_beginOfConversation)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));

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
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({},
                                            _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented)));
  actions.emplace(_action_presentation, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_presented)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));

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
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted + "&" + _fact_presented)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_presented + "&" + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));

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
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, _worldStateModification_fromStr(_fact_advertised)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_is_close),
                                              _worldStateModification_fromStr(_fact_checkedIn),
                                              _condition_fromStr(_fact_is_close)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_advertised + "&" + _fact_checkedIn),
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_is_close}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoStr(problem, domain));
}


void _checkNotInAPrecondition()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(_condition_fromStr("!" + _fact_checkedIn),
                                            _worldStateModification_fromStr(_fact_greeted)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  problem.worldState.modify(_worldStateModification_fromStr(_fact_checkedIn), problem.goalStack,
                            _emptySetOfInferences, {}, {}, now);
  assert_eq(std::string(), _lookForAnActionToDoConstStr(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
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
  cp::ProblemModification effect(_worldStateModification_fromStr(_fact_a + " & !" + _fact_b));
  effect.potentialWorldStateModification = _worldStateModification_fromStr(_fact_c);
  effect.goalsToAdd[cp::GoalStack::defaultPriority] = {_goal(_fact_d)};
  const cp::Action action(_condition_fromStr(_fact_e),
                          effect,
                          _condition_fromStr(_fact_f));
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
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr(_fact_beHappy),
                                              _worldStateModification_fromStr(_fact_beHappy)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _addGoalEvenForEmptyAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<cp::ActionId, cp::Action> actions;
  cp::Action act1Obj({}, {});
  act1Obj.effect.goalsToAddInCurrentPriority.push_back(_goal(_fact_a));
  actions.emplace(action1, act1Obj);
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  assert_true(problem.goalStack.goals().empty());
  cp::notifyActionDone(problem, domain, cp::ActionInvocationWithGoal(action1, {}, {}, 0), {});
  assert_false(problem.goalStack.goals().empty());
}

void _circularDependencies()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_greeted),
                                              _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace("check-in-pwd", cp::Action(_condition_fromStr(_fact_hasCheckInPasword),
                                             _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace("inverse-of-check-in-pwd", cp::Action(_condition_fromStr(_fact_checkedIn),
                                                        _worldStateModification_fromStr(_fact_hasCheckInPasword)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action(_condition_fromStr(_fact_beSad),
                                           _worldStateModification_fromStr("!" + _fact_beSad)));
  actions.emplace(_action_goodBoy, cp::Action(_condition_fromStr("!" + _fact_beSad),
                                              _worldStateModification_fromStr(_fact_beHappy)));

  cp::Historical historical;
  cp::Problem problem;
  _addFact(problem.worldState, _fact_beSad, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStr(problem, actions, {}, &historical));
}


void _actionWithConstantValue()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, _worldStateModification_fromStr("place=kitchen"));
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen")});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _actionWithParameterizedValue()
{
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?target"));
  cp::Action navigate({}, _worldStateModification_fromStr("place=?target", parameters));
  navigate.parameters = std::move(parameters);
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("place=kitchen")});
  assert_eq(_action_navigate + "(?target -> kitchen)", _solveStr(problem, actions));
}


void _actionWithParameterizedParameter()
{
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human"));
  cp::Action joke({}, _worldStateModification_fromStr("isHappy(?human)", parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("isHappy(1)")});
  assert_eq(_action_joke + "(?human -> 1)", _solveStr(problem, actions));
}


void _actionWithParametersInPreconditionsAndEffectsWithoutSolution()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> parameters(1, _parameter("?human"));
  cp::Action joke(_condition_fromStr("isEngaged(?human)", parameters),
                  _worldStateModification_fromStr("isHappy(?human)", parameters));
  joke.parameters = std::move(parameters);
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _addFact(problem.worldState, "isEngaged(2)", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("isHappy(1)")});
  assert_eq<std::string>("", _solveStr(problem, actions));
}

void _actionWithParametersInsideThePath()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navigateParameters(1, _parameter("?target"));
  cp::Action navigateAction({},
                            _worldStateModification_fromStr("place=?target", navigateParameters));
  navigateAction.parameters = std::move(navigateParameters);
  actions.emplace(_action_navigate, navigateAction);

  actions.emplace(_action_welcome,
                  cp::Action(_condition_fromStr("place=entrance"),
                             _worldStateModification_fromStr("welcomePeople")));

  cp::Problem problem;
  _addFact(problem.worldState, "place=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("welcomePeople")});
  assert_eq<std::string>(_action_navigate + "(?target -> entrance)" + _sep +
                         _action_welcome, _solveStr(problem, actions));
  assert_true(_hasFact(problem.worldState, "place=entrance"));
  assert_false(_hasFact(problem.worldState, "place=kitchen"));
}


void _testPersistGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_welcome, cp::Action({}, _worldStateModification_fromStr("welcomePeople")));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("welcomePeople")});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goalStack.goals().size());

  problem = cp::Problem();
  _setGoalsForAPriority(problem, {_goal("persist(welcomePeople)")});
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goalStack.goals().size());
}


void _testPersistImplyGoal()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_greeted + ", " + _fact_checkedIn + "))")});
  assert_eq<std::string>("", _solveStr(problem, actions));
  _addFact(problem.worldState, _fact_greeted, problem.goalStack);
  assert_eq<std::string>(_action_checkIn, _solveStr(problem, actions));
}


void _testImplyGoal()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_greeted + ", " + _fact_checkedIn + ")")});
  assert_eq<std::string>("", _solveStr(problem, actions));
  // It is not a persistent goal it is removed
  _addFact(problem.worldState, _fact_greeted, problem.goalStack);
  assert_eq<std::string>("", _solveStr(problem, actions));
}


void _checkPreviousBugAboutSelectingAnInappropriateAction()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action(_condition_fromStr(_fact_engagedWithUser),
                                                   _worldStateModification_fromStr(_fact_userSatisfied),
                                                   _condition_fromStr("!" + _fact_robotLearntABehavior)));
  actions.emplace(_action_checkIn, cp::Action({},
                                              _worldStateModification_fromStr("!" + _fact_robotLearntABehavior + " & " + _fact_advertised)));
  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack);
  _setGoalsForAPriority(problem, {"persist(" + _fact_userSatisfied + ")"});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
  _removeFact(problem.worldState, _fact_userSatisfied, problem.goalStack);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _dontLinkActionWithPreferredInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_askQuestion1, cp::Action({},
                                                   _worldStateModification_fromStr(_fact_userSatisfied),
                                                   _condition_fromStr(_fact_checkedIn)));
  actions.emplace(_action_checkIn, cp::Action(_condition_fromStr(_fact_engagedWithUser),
                                              _worldStateModification_fromStr(_fact_checkedIn)));
  cp::Domain domain(std::move(actions));

  cp::Historical historical;
  cp::Problem problem;
  _setFacts(problem.worldState, {_fact_engagedWithUser}, problem.goalStack);
  _setGoalsForAPriority(problem, {_fact_userSatisfied});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _checkPriorities()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_fact_greeted}}, {9, {_fact_beHappy}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _stackablePropertyOfGoals()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented)));

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted, 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted, 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  problem2.goalStack.pushFrontGoal(_goal(_fact_presented), problem2.worldState, {}, 10);
  assert_eq(_action_presentation + _sep +
            _action_goodBoy, _solveStr(problem2, actions));
}



void _doNotRemoveAGoalWithMaxTimeToKeepInactiveEqual0BelowAGoalWithACondotionNotSatisfied()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy)));
  actions.emplace(_action_presentation, cp::Action({}, _worldStateModification_fromStr(_fact_presented)));

  // Even if _fact_checkedIn has maxTimeToKeepInactive equal to 0, it is not removed because the goal with a higher priority is inactive.
  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  _addFact(problem2.worldState, _fact_presented, problem2.goalStack); // The difference here is that the condition of the first goal is satisfied
  _setGoals(problem2, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem2, actions));


  cp::Problem problem3;
  _setGoals(problem3, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  _addFact(problem3.worldState, _fact_presented, problem3.goalStack); // The difference here is that the condition is validated after the add of the goal
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem3, actions));


  cp::Problem problem4;
  _setGoals(problem4, {{10, {_goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}});
  _addFact(problem4.worldState, _fact_presented, problem4.goalStack); // Here _fact_checkedIn goal shoud be removed from the stack
  _removeFact(problem4.worldState, _fact_presented, problem4.goalStack); // The difference here is that the condition was validated only punctually
  assert_eq(_action_goodBoy, _solveStr(problem4, actions));
}



void _checkMaxTimeToKeepInactiveForGoals()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));


  cp::Problem problem;
  _setGoals(problem, {{10, {_goal(_fact_greeted), _goal(_fact_checkedIn, 60)}}}, now);
  assert_eq(_action_greet + _sep +
            _action_checkIn, _solveStr(problem, actions, now));


  cp::Problem problem2;
  _setGoals(problem2, {{10, {_goal(_fact_greeted), _goal(_fact_checkedIn, 60)}}}, now);
  now = std::make_unique<std::chrono::steady_clock::time_point>(*now + std::chrono::seconds(100));
  assert_eq(_action_greet, _solveStr(problem2, actions, now));
}



void _changePriorityOfGoal()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));

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

  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, true, problem.worldState, now);
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
  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, false, problem.worldState, now);
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
  problem.goalStack.changeGoalPriority(_fact_checkedIn, 9, true, problem.worldState, now);
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, _worldStateModification_fromStr(_fact_greeted)));
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn + "&" + _fact_punctual_p1)));
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  assert_eq<std::string>("", _solveStr(problem, domain, now));
  // Inference: if (_fact_headTouched) then remove(_fact_headTouched) and addGoal(_fact_checkedIn)
  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_headTouched),
                                          _worldStateModification_fromStr("!" + _fact_headTouched),
                                          _emptyParameters, {{{9, {_goal(_fact_checkedIn)}}}}));
  assert_eq<std::string>("", _solveStr(problem, domain, now));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, setOfInferencesMap, now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, domain, now));
}



void _checkInferencesWithImply()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_checkedIn)));

  // Inference: if (_fact_headTouched) then add(_fact_userWantsToCheckedIn) and remove(_fact_headTouched)
  cp::Domain domain(std::move(actions), {},
                    cp::Inference(_condition_fromStr(_fact_headTouched),
                                  _worldStateModification_fromStr(_fact_userWantsToCheckedIn + " & !" + _fact_headTouched)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(imply(" + _fact_userWantsToCheckedIn + ", " + _fact_checkedIn + "))")});
  assert_eq<std::string>("", _solveStr(problem, domain, now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_headTouched, problem.goalStack, setOfReferencesMap, now);
  assert_true(!_hasFact(problem.worldState, _fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, domain, now));
}


void _checkInferenceWithPunctualCondition()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn)));

  // Inference: if (_fact_punctual_headTouched) then add(_fact_userWantsToCheckedIn)
  cp::Domain domain(std::move(actions), {},
                    cp::Inference(_condition_fromStr(_fact_punctual_headTouched),
                                  _worldStateModification_fromStr(_fact_userWantsToCheckedIn)));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  assert_eq<std::string>("", _solveStr(problem, domain, now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, setOfReferencesMap, now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, now));
}


void _checkInferenceAtEndOfAPlan()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, _worldStateModification_fromStr(_fact_punctual_checkedIn)));

  cp::SetOfInferences setOfInferences;
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_headTouched),
                                             _worldStateModification_fromStr(_fact_userWantsToCheckedIn)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_checkedIn),
                                             _worldStateModification_fromStr("!" + _fact_userWantsToCheckedIn)));
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  assert_eq<std::string>("", _solveStr(problem, domain, now));
  auto& setOfReferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_punctual_headTouched, problem.goalStack, setOfReferencesMap, now);
  assert_true(!_hasFact(problem.worldState, _fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, domain, now));
}


void _checkInferenceInsideAPlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_c), _worldStateModification_fromStr(_fact_d)));
  cp::Domain domain(std::move(actions));

  {
    cp::SetOfInferences setOfInferences;
    setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_a),
                                               _worldStateModification_fromStr(_fact_b)));
    setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_b + "&" + _fact_d),
                                               _worldStateModification_fromStr(_fact_c)));
    domain.addSetOfInferences(std::move(setOfInferences));
  }

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d)});
  assert_eq<std::string>("", _solveStrConst(problem, domain));

  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_b),
                                          _worldStateModification_fromStr(_fact_c)));

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

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_c),
                                      _worldStateModification_fromStr(_fact_d)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_c + "&" + _fact_f),
                                      _worldStateModification_fromStr(_fact_e)));
  actions.emplace(action4, cp::Action(_condition_fromStr(_fact_b),
                                      _worldStateModification_fromStr(_fact_f)));
  actions.emplace(action5, cp::Action(_condition_fromStr(_fact_b),
                                      _worldStateModification_fromStr(_fact_g)));
  cp::SetOfInferences setOfInferences;
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_a),
                                             _worldStateModification_fromStr(_fact_b),
                                             _emptyParameters, {{cp::GoalStack::defaultPriority, {_goal(_fact_e)}}}));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_b),
                                             _worldStateModification_fromStr(_fact_c)));
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("imply(" + _fact_g + ", " + _fact_d + ")")});
  assert_eq<std::string>("", _solveStrConst(problem, domain));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, _fact_g, problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + _sep + action4 + _sep + action3 + _sep + action2, _solveStr(problem, domain));
}


void _testGetNotSatisfiedGoals()
{
  auto goal1 = "persist(!" + _fact_a + ")";
  auto goal2 = "persist(" + _fact_b + ")";
  auto goal3 = "imply(" + _fact_c + ", " + _fact_d + ")";
  auto goal4 = "persist(imply(!" + _fact_c + ", " + _fact_d + "))";

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

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_b)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c)));
  cp::Domain domain(std::move(actions));

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(!" + _fact_a + ")"}, now, cp::GoalStack::defaultPriority + 2);
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    _addFact(problem.worldState, _fact_a, problem.goalStack);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {"persist(" + _fact_c + ")"}, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    _addGoalsForAPriority(problem, {_fact_c}, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, {});
    _addGoalsForAPriority(problem, {_goal(_fact_b, 0)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.goalStack.pushBackGoal({_goal("persist(!" + _fact_e + ")")}, problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    problem.goalStack.pushBackGoal(_goal(_fact_c), problem.worldState, now, cp::GoalStack::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
    problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, now);
    _addGoalsForAPriority(problem, {_goal(_fact_b, 1)}, now, cp::GoalStack::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));

    _removeFact(problem.worldState, _fact_c, problem.goalStack);
    problem.goalStack.pushBackGoal(_goal(_fact_c), problem.worldState, now, cp::GoalStack::defaultPriority + 2);
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  cp::SetOfInferences setOfInferences;
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p2),
                                             _worldStateModification_fromStr(_fact_a)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p5),
                                             _worldStateModification_fromStr(_fact_punctual_p2 + "&" + _fact_punctual_p3)));
  setOfInferences.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p4),
                                             _worldStateModification_fromStr(_fact_punctual_p5 + "&" + _fact_punctual_p1)));

  std::map<cp::SetOfInferencesId, cp::SetOfInferences> setOfInferencesMap = {{"soi", setOfInferences}};
  assert_false(_hasFact(problem.worldState, _fact_a));
  _addFact(problem.worldState, _fact_punctual_p4, problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, _fact_a));
}



void _oneStepTowards()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification greetPbModification;
  greetPbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_greeted);
  actions.emplace(_action_greet, cp::Action({}, greetPbModification));
  actions.emplace(_action_goodBoy, cp::Action({}, _worldStateModification_fromStr(_fact_beHappy)));
  static const std::string actionb = "actionb";
  actions.emplace(actionb, cp::Action({}, _worldStateModification_fromStr(_fact_b)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  auto implyGoal = _goal("persist(imply(" + _fact_a + ", " + _fact_b + "))", 0);
  _setGoals(problem, {{11, {implyGoal}},
                      {10, {_goal("oneStepTowards(" + _fact_greeted + ")", 0)}},
                      {9, {_goal(_fact_checkedIn, 0), _goal(_fact_beHappy)}}}, now);
  problem.goalStack.removeFirstGoalsThatAreAlreadySatisfied(problem.worldState, now);
  assert_eq(_action_greet, _lookForAnActionToDoStr(problem, domain, now));
  assert_eq(_action_greet, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, now));
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, now));
  _addFact(problem.worldState, _fact_a, problem.goalStack);
  assert_eq(actionb, _lookForAnActionToDoStr(problem, domain, now));
  assert_eq<std::string>(actionb + _sep + _action_goodBoy, _solveStrConst(problem, domain));
  assert_eq<std::string>(implyGoal.toStr() + _sep + _fact_beHappy, _getGoalsDoneDuringAPlannificationConst(problem, domain));
}


void _infrenceLinksFromManyInferencesSets()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification pbModification;
  pbModification.potentialWorldStateModification = _worldStateModification_fromStr(_fact_d);
  actions.emplace(action1, cp::Action({}, pbModification));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_c)));
  cp::Domain domain(std::move(actions));

  assert_true(cp::GoalStack::defaultPriority >= 1);
  auto lowPriority = cp::GoalStack::defaultPriority - 1;
  domain.addSetOfInferences(cp::Inference(_condition_fromStr(_fact_punctual_p2),
                                          {}, _emptyParameters,
                                          {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")")}}}));
  cp::Problem problem;
  _setGoals(problem, {{lowPriority, {_goal("oneStepTowards(" + _fact_d + ")", 0)}}});

  {
    cp::SetOfInferences setOfInferences2;
    setOfInferences2.addInference(cp::Inference(_condition_fromStr(_fact_punctual_p1),
                                                _worldStateModification_fromStr(_fact_b + "&" + _fact_punctual_p2)));
    setOfInferences2.addInference(cp::Inference(_condition_fromStr(_fact_b),
                                                {}, _emptyParameters,
                                                {{cp::GoalStack::defaultPriority, {_goal("oneStepTowards(" + _fact_c + ")")}}}));
    domain.addSetOfInferences(setOfInferences2);
  }

  auto& setOfInferencesMap = domain.getSetOfInferences();
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
  _addFact(problem.worldState, _fact_punctual_p1, problem.goalStack, setOfInferencesMap, now);
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
}


void _factValueModification()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("!" + _fact_b)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoals(problem, {{10, {_goal("persist(imply(" + _fact_a + "=a, " + "!" + _fact_b + "))", 0)}}});

  _addFact(problem.worldState, _fact_b, problem.goalStack);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, now));
  _addFact(problem.worldState, _fact_a + "=a", problem.goalStack);
  assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));
  _addFact(problem.worldState, _fact_a + "=b", problem.goalStack);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, now));
}


void _removeGoaWhenAnActionFinishesByAddingNewGoals()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  std::map<cp::ActionId, cp::Action> actions;
  cp::ProblemModification wm(_worldStateModification_fromStr(_fact_a));
  wm.goalsToAddInCurrentPriority.push_back(_goal(_fact_b, 0));
  actions.emplace(action1, cp::Action({}, wm));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_b)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem.goalStack);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  _setGoals(problem, {{27, {_fact_a}}}, now);

  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("set(location(me), location(object))"));
  actions.emplace(_action_navigate, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(object)=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("location(me)=kitchen")});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _forAllWsModification()
{
  const std::string action1 = "action1";
  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, _worldStateModification_fromStr("forAll(?obj, grab(me, ?obj), set(location(?obj), location(me)))"));
  actions.emplace(action1, navAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(object1)=kitchen", problem.goalStack);
  _addFact(problem.worldState, "grab(me, object1)", problem.goalStack);
  _addFact(problem.worldState, "grab(me, object2)", problem.goalStack);

  _setGoalsForAPriority(problem, {_goal("location(object2)=corridor")});
  assert_eq(action1, _solveStr(problem, actions));
}


void _actionNavigationAndGrabObjectWithParameters()
{
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)"))->toStr());
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)")});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)", _solveStr(problem, actions));
}


void _actionNavigationAndGrabObjectWithParameters2()
{
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(?object), location(me))", grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  assert_eq<std::string>("kitchen", problem.worldState.getFactFluent(_fact("location(sweets)"))->toStr());
  _setGoalsForAPriority(problem, {_goal("grab(me, sweets)")});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets)", _solveStr(problem, actions));
}



void _moveObject()
{
  const std::string actionNavigate2 = "actionNavigate2";
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("location(me)=?targetLocation", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> nav2Parameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Action navAction2(_condition_fromStr("grab(me, ?object)", nav2Parameters),
                        _worldStateModification_fromStr("location(me)=?targetLocation & location(?object)=?targetLocation", nav2Parameters));
  navAction2.parameters = std::move(nav2Parameters);
  actions.emplace(actionNavigate2, navAction2);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(location(me), location(?object))", grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  _addFact(problem.worldState, "location(me)=corridor", problem.goalStack);
  _addFact(problem.worldState, "location(sweets)=kitchen", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal("location(sweets)=bedroom")});
  assert_eq<std::string>(_action_navigate + "(?targetLocation -> kitchen), " + _action_grab + "(?object -> sweets), " +
                         actionNavigate2 + "(?object -> sweets, ?targetLocation -> bedroom)", _solveStr(problem, actions));
}


void _moveAndUngrabObject()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction({}, _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters(1, _parameter("?object"));
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters(1, _parameter("?object"));
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::Problem problem;
  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", inferenceParameters),
                          _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)")});

  assert_eq(_action_navigate + "(?targetLocation -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "locationOfObj(sweets)=kitchen"));
}


void _failToMoveAnUnknownObject()
{
  const std::string actionWhereIsObject = "actionWhereIsObject";
  const std::string actionLeavePod = "actionLeavePod";
  const std::string actionRanomWalk = "actionRanomWalk";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters(1, _parameter("?targetLocation"));
  cp::Action navAction(_condition_fromStr("!isLost & !charging(me)", navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetLocation", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  cp::Action randomWalkAction(_condition_fromStr("!charging(me)"),
                       _worldStateModification_fromStr("!isLost"));
  actions.emplace(actionRanomWalk, randomWalkAction);

  cp::Action leavePodAction({}, _worldStateModification_fromStr("!charging(me)"));
  actions.emplace(actionLeavePod, leavePodAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object"), _parameter("?aLocation")};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObj(?object)=*", whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObj(?object)=?aLocation", whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object")};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObj(?object))", grabParameters),
                        _worldStateModification_fromStr("grab(me, ?object)", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object")};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me, ?object)", ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?targetLocation"), _parameter("?object")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?targetLocation & grab(me, ?object)", inferenceParameters),
                          _worldStateModification_fromStr("locationOfObj(?object)=?targetLocation", inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)")});

  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "charging(me)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, "locationOfObj(sweets)=kitchen", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("locationOfObj(sweets)=bedroom & !grab(me, sweets)")});
  assert_eq(actionLeavePod, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetLocation -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _completeMovingObjectScenario()
{
  const std::string actionWhereIsObject = "actionWhereIsObject";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> navParameters{_parameter("?targetPlace")};
  cp::Action navAction(_condition_fromStr("!lost(me) & !pathIsBlocked", navParameters),
                       _worldStateModification_fromStr("locationOfRobot(me)=?targetPlace", navParameters));
  navAction.parameters = std::move(navParameters);
  actions.emplace(_action_navigate, navAction);

  std::vector<cp::Parameter> grabParameters{_parameter("?object")};
  cp::Action grabAction(_condition_fromStr("equals(locationOfRobot(me), locationOfObject(?object)) & not(exists(o, grab(me)=o))", grabParameters),
                        _worldStateModification_fromStr("grab(me)=?object", grabParameters));
  grabAction.parameters = std::move(grabParameters);
  actions.emplace(_action_grab, grabAction);

  std::vector<cp::Parameter> ungrabParameters{_parameter("?object")};
  cp::Action ungrabAction({}, _worldStateModification_fromStr("!grab(me)=?object", ungrabParameters));
  ungrabAction.parameters = std::move(ungrabParameters);
  actions.emplace(_action_ungrab, ungrabAction);

  std::vector<cp::Parameter> whereIsObjectParameters{_parameter("?object"), _parameter("?aLocation")};
  cp::Action whereIsObjectAction(_condition_fromStr("!locationOfObject(?object)=*", whereIsObjectParameters),
                                 _worldStateModification_fromStr("locationOfObject(?object)=?aLocation", whereIsObjectParameters));
  whereIsObjectAction.parameters = std::move(whereIsObjectParameters);
  actions.emplace(actionWhereIsObject, whereIsObjectAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object"), _parameter("?location")};
  cp::Inference inference(_condition_fromStr("locationOfRobot(me)=?location & grab(me)=?object", inferenceParameters),
                          _worldStateModification_fromStr("locationOfObject(?object)=?location", inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets")});

  assert_eq(actionWhereIsObject + "(?aLocation -> bedroom, ?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  auto& setOfInferencesMap = domain.getSetOfInferences();
  _addFact(problem.worldState, "locationOfObject(sweets)=kitchen", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("locationOfObject(sweets)=bedroom & !grab(me)=sweets")});
  assert_eq(_action_navigate + "(?targetPlace -> kitchen)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_grab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_navigate + "(?targetPlace -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(_action_ungrab + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal("locationOfObject(bottle)=entrance & !grab(me)=bottle")});
  assert_eq(actionWhereIsObject + "(?aLocation -> entrance, ?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}

void _inferenceWithANegatedFactWithParameter()
{
  const std::string actionUngrabLeftHand = "actionUngrabLeftHand";
  const std::string actionUngrabRightHand = "actionUngrabRightHand";
  const std::string actionUngrabBothHands = "actionUngrabBothHands";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> ungrabLeftParameters{_parameter("?object")};
  cp::Action ungrabLeftAction(_condition_fromStr("!hasTwoHandles(?object)", ungrabLeftParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object", ungrabLeftParameters));
  ungrabLeftAction.parameters = std::move(ungrabLeftParameters);
  actions.emplace(actionUngrabLeftHand, ungrabLeftAction);

  std::vector<cp::Parameter> ungrabRightParameters{_parameter("?object")};
  cp::Action ungrabRightAction(_condition_fromStr("!hasTwoHandles(?object)", ungrabRightParameters),
                               _worldStateModification_fromStr("!grabRightHand(me)=?object", ungrabRightParameters));
  ungrabRightAction.parameters = std::move(ungrabRightParameters);
  actions.emplace(actionUngrabRightHand, ungrabRightAction);

  std::vector<cp::Parameter> ungrabBothParameters{_parameter("?object")};
  cp::Action ungrabBothAction(_condition_fromStr("hasTwoHandles(?object)", ungrabBothParameters),
                              _worldStateModification_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", ungrabBothParameters));
  ungrabBothAction.parameters = std::move(ungrabBothParameters);
  actions.emplace(actionUngrabBothHands, ungrabBothAction);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object")};
  cp::Inference inference(_condition_fromStr("!grabLeftHand(me)=?object & !grabRightHand(me)=?object", inferenceParameters),
                          _worldStateModification_fromStr("!grab(me, ?object)", inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, "hasTwoHandles(sweets)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grabLeftHand(me)=sweets", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grabRightHand(me)=sweets", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grab(me, sweets)", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, sweets)")});

  assert_eq(actionUngrabBothHands + "(?object -> sweets)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, sweets)"));

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, bottle)")});
  assert_eq(actionUngrabLeftHand + "(?object -> bottle)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());

  _addFact(problem.worldState, "grabLeftHand(me)=bottle", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grab(me, bottle)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grabRightHand(me)=glass", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "grab(me, glass)", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal("!grab(me, glass)")});
  assert_eq(actionUngrabRightHand + "(?object -> glass)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, "grab(me, glass)"));
}


void _actionWithANegatedFactNotTriggeredIfNotNecessary()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr("!" + _fact_a),
                                      _condition_fromStr(_fact_c + " & " + _fact_d)));

  actions.emplace(action2, cp::Action(_condition_fromStr("!" + _fact_a + " & " + _fact_e),
                                      _worldStateModification_fromStr(_fact_b)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_e)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_c, problem.goalStack);
  _addFact(problem.worldState, _fact_d, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_b)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}



void _useTwoTimesAnInference()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inferenceParameters{_parameter("?object")};
  cp::Inference inference(_condition_fromStr(_fact_a + " & " + _fact_b + "(?object)", inferenceParameters),
                          _worldStateModification_fromStr(_fact_c + "(?object)", inferenceParameters));
  inference.parameters = std::move(inferenceParameters);
  setOfInferences.addInference(inference);

  std::map<std::string, cp::Action> actions;
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(obj2)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj1)"));
  assert_true(_hasFact(problem.worldState, _fact_c + "(obj2)"));
}


void _linkWithAnyValueInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action(_condition_fromStr("!" + _fact_a + "=*"),
                                      _worldStateModification_fromStr(_fact_b)));

  std::vector<cp::Parameter> act2Parameters{_parameter("?aVal")};
  cp::Action act2({}, _worldStateModification_fromStr("!" + _fact_a + "=?aVal", act2Parameters));
  act2.parameters = std::move(act2Parameters);
  actions.emplace(action2, act2);

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "=toto", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_b)});
  assert_eq(action2 + "(?aVal -> toto)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _removeAFactWithAnyValue()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + " & !" + _fact_b + "=*")));
  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "=toto", problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_a)});
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_b + "=toto"));
}



void _notDeducePathIfTheParametersOfAFactAreDifferents()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "(1)"),
                                      _condition_fromStr(_fact_c)));
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "(2)"),
                                      _worldStateModification_fromStr(_fact_b)));
  actions.emplace(action3, cp::Action(_condition_fromStr(_fact_b),
                                      _worldStateModification_fromStr(_fact_d)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "(2)", problem.goalStack);
  _addFact(problem.worldState, _fact_c, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_d)});

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.actionId);
}


void _checkPreferInContext()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a), _condition_fromStr(_fact_b)));
  actions.emplace(action2, cp::Action({}, _worldStateModification_fromStr(_fact_a)));

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
  std::map<std::string, cp::Action> actions;
  auto action1Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a), _condition_fromStr(_fact_b));
  action1Obj.highImportanceOfNotRepeatingIt = true;
  actions.emplace(action1, action1Obj);
  auto action2Obj = cp::Action({}, _worldStateModification_fromStr(_fact_a));
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action({},
                                      _worldStateModification_fromStr(_fact_a + "=a")));

  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "!=b & " + _fact_d),
                                      _worldStateModification_fromStr(_fact_c)));

  actions.emplace(action3, cp::Action({},
                                      _worldStateModification_fromStr(_fact_d),
                                      _condition_fromStr(_fact_e)));

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack);
  _setGoalsForAPriority(problem, {_goal(_fact_c)});

  assert_eq(action3, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _negatedFactValueInWorldState()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a + "!=b"),
                                      _worldStateModification_fromStr(_fact_b)));
  cp::Domain domain(std::move(actions));

  {
    cp::Problem problem;
    _setGoalsForAPriority(problem, {_goal(_fact_b)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=b", problem.goalStack);
    _setGoalsForAPriority(problem, {_goal(_fact_b)});
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "=c", problem.goalStack);
    _setGoalsForAPriority(problem, {_goal(_fact_b)});

    assert_eq<std::string>(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  }

  {
    cp::Problem problem;
    _addFact(problem.worldState, _fact_a + "!=b", problem.goalStack);
    _setGoalsForAPriority(problem, {_goal(_fact_b)});

    assert_eq<std::string>(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  }
}


void _problemThatUseADomainThatChangedSinceLastUsage()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  actions.emplace(action1, cp::Action(_condition_fromStr(_fact_a),
                                      _worldStateModification_fromStr(_fact_b)));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b)});
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, now)); // set problem cache about domain

  domain.addAction(action2, cp::Action({},
                                       _worldStateModification_fromStr(_fact_a)));

  _setGoalsForAPriority(problem, {_goal(_fact_b)});
  assert_eq<std::string>(action2, _lookForAnActionToDoStr(problem, domain, now)); // as domain as changed since last time the problem cache should be regenerated
}


void _checkFilterFactInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "(?obj)", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj"), _parameter("?loc")};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "(?obj)", act2Parameters),
                        _worldStateModification_fromStr(_fact_c, act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);
  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(obj1, loc2)", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal(_fact_c)});
  assert_eq<std::string>(action1 + "(?obj -> obj1)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>(action2 + "(?loc -> loc1, ?obj -> obj1)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _checkFilterFactInConditionAndThenPropagate()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?obj"), _parameter("?loc")};
  cp::Action actionObj2(_condition_fromStr(_fact_b + "(?obj, ?loc) & " + _fact_a + "=?obj", act2Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?loc)", act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);


  std::vector<cp::Parameter> act3Parameters{_parameter("?loc")};
  cp::Action actionObj3(_condition_fromStr(_fact_c + "(?loc)", act3Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?loc)", act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_b + "(obj1, loc1)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(obj2, loc2)", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(loc2)")});
  assert_eq(action1 + "(?obj -> obj2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2 + "(?loc -> loc2, ?obj -> obj2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action3 + "(?loc -> loc2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}



void _satisfyGoalWithSuperiorOperator()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + "=100")));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_a + "=10", problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal(_fact_a + ">50")});
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}



void _checkOutputValueOfLookForAnActionToDo()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a)});

  {
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, now, nullptr, &lookForAnActionOutputInfos);
    assert(!res.empty());
    assert_eq(cp::PlannerStepType::IN_PROGRESS, lookForAnActionOutputInfos.getType());
    assert_eq<std::size_t>(0, lookForAnActionOutputInfos.nbOfSatisfiedGoals());
  }

  {
    _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, now);
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, now, nullptr, &lookForAnActionOutputInfos);
    assert(res.empty());
    assert_eq(cp::PlannerStepType::FINISHED_ON_SUCCESS, lookForAnActionOutputInfos.getType());
    assert_eq<std::size_t>(1, lookForAnActionOutputInfos.nbOfSatisfiedGoals());
  }

  {
    _setGoalsForAPriority(problem, {_goal(_fact_b)});
    cp::LookForAnActionOutputInfos lookForAnActionOutputInfos;
    auto res = cp::planForMoreImportantGoalPossible(problem, domain, true, now, nullptr, &lookForAnActionOutputInfos);
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + "=val1"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + "=val2"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action3, actionObj3);

  cp::Action actionObj4(_condition_fromStr(_fact_a + "=val3 & !" + _fact_c),
                        _worldStateModification_fromStr("!" + _fact_d + " & " + _fact_f));
  actions.emplace(action4, actionObj4);

  cp::Action actionObj5(_condition_fromStr(_fact_a + "=val4"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action5, actionObj5);

  cp::Action actionObj6(_condition_fromStr(_fact_b + " & !" + _fact_d),
                        _worldStateModification_fromStr(_fact_e));
  actions.emplace(action6, actionObj6);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal(_fact_e)});

  assert_eq(action1 + "(?obj -> val3)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action4, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> val1)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action6, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
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
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1({}, _worldStateModification_fromStr(_fact_a + "=?obj", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + "=val1"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + "=val2"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action3, actionObj3);

  cp::Action actionObj4(_condition_fromStr(_fact_a + "=val3 & !" + _fact_c),
                        _worldStateModification_fromStr("!" + _fact_d + " & " + _fact_f));
  actions.emplace(action4, actionObj4);

  cp::Action actionObj5(_condition_fromStr(_fact_a + "=val4"),
                        _worldStateModification_fromStr(_fact_b + " & " + _fact_c));
  actions.emplace(action5, actionObj5);

  cp::Action actionObj6(_condition_fromStr(_fact_b + " & !" + _fact_d + " & " + _fact_g),
                        _worldStateModification_fromStr(_fact_e));
  actions.emplace(action6, actionObj6);

  cp::Action actionObj7(_condition_fromStr(_fact_f),
                        _worldStateModification_fromStr(_fact_g));
  actions.emplace(action7, actionObj7);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_d, problem.goalStack, setOfInferencesMap, now);
  _setGoalsForAPriority(problem, {_goal(_fact_e)});

  assert_eq(action1 + "(?obj -> val3)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());

  assert_eq<std::string>(action1 + "(?obj -> val3)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_eq<std::string>(action4, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_eq<std::string>(action7 + _sep + action1 + "(?obj -> val1)", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_eq<std::string>(action2, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_eq<std::string>(action6, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_eq<std::string>("", _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
}



void _checkOverallEffectDuringParallelisation()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::Action actionObj1(_condition_fromStr("!" + _fact_d),
                        _worldStateModification_fromStr(_fact_a + " & !" + _fact_d));
  actionObj1.effect.worldStateModificationAtStart = _worldStateModification_fromStr(_fact_d + " & " + _fact_e);
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr("!" + _fact_d),
                        _worldStateModification_fromStr(_fact_b + " & !" + _fact_d));
  actionObj2.effect.worldStateModificationAtStart = _worldStateModification_fromStr(_fact_d);
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + " & " + _fact_b),
                        _worldStateModification_fromStr(_fact_c));
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c)});

  assert_eq<std::string>(action1, _lookForAnActionToDoInParallelThenNotifyToStr(problem, domain, now));
  assert_true(_hasFact(problem.worldState, _fact_e));
}

void _checkSimpleExists()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(?l))"),
                        _worldStateModification_fromStr(_fact_b));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1, _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _checkExistsWithActionParameterInvolved()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> actionParameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(?l, ?obj))", actionParameters),
                        _worldStateModification_fromStr(_fact_b));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b)});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(kitchen, pen)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _checkExistsWithManyFactsInvolved()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> actionParameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", actionParameters),
                        _worldStateModification_fromStr(_fact_c, actionParameters));
  actionObj1.parameters = std::move(actionParameters);
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c)});

  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?obj -> bottle)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _doAnActionToSatisfyAnExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)")});

  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _checkForAllEffectAtStart()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self, ?l) & " + _fact_b + "(?obj, ?l))", act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self, ?loc)", act2Parameters));
  actionObj2.effect.worldStateModificationAtStart = _worldStateModification_fromStr("forall(?l, when(" + _fact_a + "(self, ?l), not(" + _fact_a + "(self, ?l))))", act2Parameters);
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(mouse)")});

  _addFact(problem.worldState, _fact_a + "(self, entrance)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(bottle, kitchen)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(mouse, bedroom)", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(pen, livingroom)", problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, _fact_a + "(self, entrance)"));
  assert_eq(action2 + "(?loc -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_false(_hasFact(problem.worldState, _fact_a + "(self, entrance)")); // removed by the effect at start of action2
  assert_eq(action1 + "(?obj -> mouse)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _existsWithValue()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  std::vector<cp::Parameter> act1Parameters{_parameter("?obj")};
  cp::Action actionObj1(_condition_fromStr("exists(?l, " + _fact_a + "(self)=?l & " + _fact_b + "(?obj)=?l)", act1Parameters),
                        _worldStateModification_fromStr(_fact_c + "(?obj)", act1Parameters));
  actionObj1.parameters = std::move(act1Parameters);
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr(_fact_a + "(self)=?loc", act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c + "(pen)")});

  _addFact(problem.worldState, _fact_b + "(pen)=livingroom", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action2 + "(?loc -> livingroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action1 + "(?obj -> pen)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _notExists()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::Action actionObj1(_condition_fromStr("not(exists(l, " + _fact_a + "(self, l)))"),
                        _worldStateModification_fromStr(_fact_b));
  actions.emplace(action1, actionObj1);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b)});

  assert_eq(action1, _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, now);
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _actionToSatisfyANotExists()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::Action actionObj1(_condition_fromStr("not(busy(spec_rec)) & not(exists(?l, " + _fact_a + "(self, ?l)))"),
                        _worldStateModification_fromStr("not(busy(spec_rec)) & " + _fact_b));
  actions.emplace(action1, actionObj1);

  std::vector<cp::Parameter> act2Parameters{_parameter("?loc")};
  cp::Action actionObj2({}, _worldStateModification_fromStr("!" + _fact_a + "(self, ?loc)", act2Parameters));
  actionObj2.parameters = std::move(act2Parameters);
  actions.emplace(action2, actionObj2);

  std::vector<cp::Parameter> act3Parameters{_parameter("?r")};
  cp::Action actionObj3({}, _worldStateModification_fromStr("not(busy(?r))", act3Parameters));
  actionObj3.parameters = std::move(act3Parameters);
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_b)});
  _addFact(problem.worldState, "busy(spec_rec)", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action3 + "(?r -> spec_rec)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "(self, kitchen)", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action2 + "(?loc -> kitchen)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _orInCondition()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::Action actionObj1(_condition_fromStr(_fact_a + " & " + _fact_b),
                        _worldStateModification_fromStr(_fact_c));
  actions.emplace(action1, actionObj1);

  cp::Action actionObj2(_condition_fromStr(_fact_a + " | " + _fact_b),
                        _worldStateModification_fromStr(_fact_c));
  actions.emplace(action2, actionObj2);

  cp::Action actionObj3(_condition_fromStr(_fact_a + " & " + _fact_b),
                        _worldStateModification_fromStr(_fact_c));
  actions.emplace(action3, actionObj3);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c)});
  _addFact(problem.worldState, _fact_a, problem.goalStack, setOfInferencesMap, now);

  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _derivedPredicate()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr(_fact_a + "(?o)" + " & " + _fact_b + "(?o)", derPred1Parameters),
                                         _fact(_fact_c + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  std::vector<cp::Parameter> derPred2Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate2(_condition_fromStr(_fact_a + "(?o)" + " | " + _fact_b + "(?o)", derPred2Parameters),
                                         _fact(_fact_d + "(?o)", derPred2Parameters), derPred2Parameters);
  for (auto& currInference : derivedPredicate2.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, _fact_e, problem.goalStack, setOfInferencesMap, now);

  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_a + "(book)", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_b + "(book)", problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)"));
  assert_false(_hasFact(problem.worldState, _fact_d + "(titi)"));
  _removeFact(problem.worldState, _fact_a + "(book)", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  _addFact(problem.worldState, _fact_a + "(titi)", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, _fact_c + "(book)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(book)"));
  assert_false(_hasFact(problem.worldState, _fact_c + "(titi)"));
  assert_true(_hasFact(problem.worldState, _fact_d + "(titi)"));
}


void _assignAnotherValueToSatisfyNotGoal()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr(_fact_a + "=toto")));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi")});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _assignUndefined()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, _worldStateModification_fromStr("assign(" + _fact_a + ", undefined)")));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal("!" + _fact_a + "=titi")});

  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_a + "=titi", problem.goalStack, setOfInferencesMap, now);
  assert_eq<std::size_t>(1u, problem.worldState.facts().size());
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::size_t>(0u, problem.worldState.facts().size()); // because assign undefined is done like a fact removal
}


void _assignAFact()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=valGoal")});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _assignAFactToAction()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  actions.emplace(action2, cp::Action(_condition_fromStr(_fact_a + "=valGoal"),
                                      _worldStateModification_fromStr(_fact_c)));

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_c)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}


void _assignAFactThenCheckEqualityWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", action2Parameters),
                        _worldStateModification_fromStr(_fact_d, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}



void _assignAFactThenCheckExistWithAnotherFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  cp::Action action2Obj(_condition_fromStr("exists(?pc, =(" + _fact_a + ", " + _fact_c + "(?pc)))"),
                        _worldStateModification_fromStr(_fact_d));
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d)});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _existWithEqualityInInference()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o")};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr("exists(?pc, =(" + _fact_c + "(?pc), " + _fact_a + "(?o)))", derPred1Parameters),
                                         _fact(_fact_b + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);


  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)")});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)")});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
}


void _existWithEqualityInInference_withEqualityInverted()
{
  const std::string action1 = "action1";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?o")};
  cp::Action action1Obj(_condition_fromStr(_fact_b + "(?o)", action1Parameters),
                        _worldStateModification_fromStr(_fact_d + "(?o)", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?o")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr("exists(?pc, =(" + _fact_a + "(?o), " + _fact_c + "(?pc)))", derPred1Parameters),
                                         _fact(_fact_b + "(?o)", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);


  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p2)")});
  _addFact(problem.worldState, _fact_a + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_a + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_a + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _removeFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _addFact(problem.worldState, _fact_c + "(pc1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  assert_eq(action1 + "(?o -> p2)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
  _setGoalsForAPriority(problem, {_goal(_fact_d + "(p1)")});
  assert_eq(action1 + "(?o -> p1)", _lookForAnActionToDoConst(problem, domain, now).actionInvocation.toStr());
}



void _fixInferenceWithFluentInParameter()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + "(titi), " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);
  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_e + "(titi), " + _fact_c + "(?pc))", action2Parameters),
                        _worldStateModification_fromStr(_fact_d, action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> derPred1Parameters{_parameter("?a"), _parameter("?v")};
  cp::DerivedPredicate derivedPredicate1(_condition_fromStr(_fact_a + "(?a)=?v", derPred1Parameters),
                                         _fact(_fact_e + "(?a)=?v", derPred1Parameters), derPred1Parameters);
  for (auto& currInference : derivedPredicate1.toInferences({}, {}))
    setOfInferences.addInference(currInference);

  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_d)});
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDo(problem, domain, now).actionInvocation.toStr());
}



void _assignAFactTwoTimesInTheSamePlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_a + "=kitchen")});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}


void _checkTwoTimesTheEqualityOfAFact()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  const std::string action3 = "action3";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  std::vector<cp::Parameter> action1Parameters{_parameter("?p")};
  cp::Action action1Obj({}, _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_b + "(?p))", action1Parameters));
  action1Obj.parameters = std::move(action1Parameters);
  actions.emplace(action1, action1Obj);

  std::vector<cp::Parameter> action2Parameters{_parameter("?pc")};
  cp::Action action2Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_c + "(?pc))", action2Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_a + ", " + _fact_d + "(?pc))", action2Parameters));
  action2Obj.parameters = std::move(action2Parameters);
  actions.emplace(action2, action2Obj);

  std::vector<cp::Parameter> action3Parameters{_parameter("?tt")};
  cp::Action action3Obj(_condition_fromStr("=(" + _fact_a + ", " + _fact_e + "(?tt))", action3Parameters),
                        _worldStateModification_fromStr("assign(" + _fact_f + ", ?tt)", action3Parameters));
  action3Obj.parameters = std::move(action3Parameters);
  actions.emplace(action3, action3Obj);

  cp::Domain domain(std::move(actions));
  auto& setOfInferencesMap = domain.getSetOfInferences();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_goal(_fact_f + "=bedroom")});
  _addFact(problem.worldState, _fact_b + "(p1)=aVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_b + "(p3)=anotherVal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc1)=v1", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc2)=valGoal", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_c + "(pc3)=v3", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_d + "(pc2)=kitchen", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, _fact_e + "(bedroom)=kitchen", problem.goalStack, setOfInferencesMap, now);

  assert_eq(action1 + "(?p -> p2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action2 + "(?pc -> pc2)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq(action3 + "(?tt -> bedroom)", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInvocation.toStr());
}



void _inferenceToRemoveAFactWithoutFluent()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;

  cp::SetOfInferences setOfInferences;
  std::vector<cp::Parameter> inf1Parameters{_parameter("?l")};
  cp::Inference inference1(_condition_fromStr("locationOfRobot(me)=?l", inf1Parameters),
                          _worldStateModification_fromStr("robotAt(me, ?l)", inf1Parameters));
  inference1.parameters = std::move(inf1Parameters);
  setOfInferences.addInference(inference1);

  std::vector<cp::Parameter> inf2Parameters{_parameter("?l")};
  cp::Inference inference2(_condition_fromStr("exists(?loc, locationOfRobot(me)=?loc & within(?loc)=?l)", inf2Parameters),
                          _worldStateModification_fromStr("robotAt(me, ?l)", inf2Parameters));
  inference2.parameters = std::move(inf2Parameters);
  setOfInferences.addInference(inference2);

  std::vector<cp::Parameter> inf3Parameters{_parameter("?l")};
  cp::Inference inference3(_condition_fromStr("!locationOfRobot(me)=?l", inf3Parameters),
                          _worldStateModification_fromStr("forall(?ll, robotAt(me, ?ll), !robotAt(me, ?ll))", inf3Parameters));
  inference3.parameters = std::move(inf3Parameters);
  setOfInferences.addInference(inference3);
  cp::Domain domain(std::move(actions), {}, std::move(setOfInferences));

  auto& setOfInferencesMap = domain.getSetOfInferences();

  cp::Problem problem;
  _addFact(problem.worldState, "within(house1)=city", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "within(house2)=city", problem.goalStack, setOfInferencesMap, now);
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _removeFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=house1", problem.goalStack, setOfInferencesMap, now);
  assert_true(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=city", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_true(_hasFact(problem.worldState, "robotAt(me, city)"));
  _addFact(problem.worldState, "locationOfRobot(me)=anotherCity", problem.goalStack, setOfInferencesMap, now);
  assert_false(_hasFact(problem.worldState, "robotAt(me, house1)"));
  assert_false(_hasFact(problem.worldState, "robotAt(me, city)"));
}

}




void test_plannerWithoutTypes()
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
  _satisfyGoalWithSuperiorOperator();
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
