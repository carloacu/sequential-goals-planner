#include <contextualplanner/contextualplanner.hpp>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/trackers/goalsremovedtracker.hpp>
#include <contextualplanner/util/print.hpp>
#include <iostream>
#include <assert.h>
#include "test_arithmeticevaluator.hpp"
#include "docexamples/test_planningDummyExample.hpp"
#include "docexamples/test_planningExampleWithAPreconditionSolve.hpp"


namespace
{
const std::string _sep = ", ";

const std::string _fact_a = "fact_a";
const std::string _fact_b = "fact_b";
const std::string _fact_c = "fact_c";
const std::string _fact_d = "fact_d";
const std::string _fact_e = "fact_e";
const std::string _fact_f = "fact_f";
const std::string _fact_g = "fact_g";
const std::string _fact_unreachable_u1 = cp::Fact::unreachablePrefix + "fact_u1";
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
const std::string _fact_askAllTheQuestions = "ask_all_the_questions";
const std::string _fact_finishToAskQuestions = "finished_to_ask_questions";
const std::string _fact_engagedWithUser = "engaged_with_user";
const std::string _fact_userSatisfied = "user_satisfied";
const std::string _fact_robotLearntABehavior = "robot_learnt_a_behavior";
const std::string _fact_headTouched = "head_touched";
const std::string _fact_punctual_headTouched = cp::Fact::punctualPrefix + "head_touched";
const std::string _fact_punctual_checkedIn = cp::Fact::punctualPrefix + "checked_in";

const std::string _action_presentation = "presentation";
const std::string _action_askQuestion1 = "ask_question_1";
const std::string _action_askQuestion2 = "ask_question_2";
const std::string _action_finisehdToAskQuestions = "finish_to_ask_questions";
const std::string _action_sayQuestionBilan = "say_question_bilan";
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


std::string _listOfActionsToStr(const std::list<cp::ActionInstance>& pActions)
{
  auto size = pActions.size();
  if (size == 1)
    return pActions.front().toStr();
  std::string res;
  bool firstIteration = true;
  for (const auto& currAction : pActions)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += _sep;
    res += currAction.toStr();
  }
  return res;
}


std::string _solveStrConst(const cp::Problem& pProblem,
                           const std::map<std::string, cp::Action>& pActions,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  cp::Domain domain(pActions);
  return _listOfActionsToStr(cp::lookForResolutionPlan(problem, domain, {}, pGlobalHistorical));
}

std::string _solveStr(cp::Problem& pProblem,
                      const std::map<std::string, cp::Action>& pActions,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions);
  return _listOfActionsToStr(cp::lookForResolutionPlan(pProblem, domain, pNow, pGlobalHistorical));
}

cp::OneStepOfPlannerResult _lookForAnActionToDo(cp::Problem& pProblem,
                                                const cp::Domain& pDomain,
                                                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                                const cp::Historical* pGlobalHistorical = nullptr)
{
  auto oneStepOfPlannerResultPtr = cp::lookForAnActionToDo(pProblem, pDomain, pNow, pGlobalHistorical);
  if (oneStepOfPlannerResultPtr)
    return *oneStepOfPlannerResultPtr;
  return cp::OneStepOfPlannerResult("", {}, cp::Goal("noGoal"), 0);
}

cp::OneStepOfPlannerResult _lookForAnActionToDoConst(const cp::Problem& pProblem,
                                                     const cp::Domain& pDomain,
                                                     const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                                     const cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  auto oneStepOfPlannerResultPtr = cp::lookForAnActionToDo(problem, pDomain, pNow, pGlobalHistorical);
  if (oneStepOfPlannerResultPtr)
    return *oneStepOfPlannerResultPtr;
  return cp::OneStepOfPlannerResult("", {}, cp::Goal("noGoal"), 0);
}

std::string _lookForAnActionToDoStr(cp::Problem& pProblem,
                                    const cp::Domain& pDomain,
                                    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                    const cp::Historical* pGlobalHistorical = nullptr)
{
  return _lookForAnActionToDo(pProblem, pDomain, pNow, pGlobalHistorical).actionInstance.toStr();
}

std::string _lookForAnActionToDoConstStr(const cp::Problem& pProblem,
                                         const cp::Domain& pDomain,
                                         const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                                         const cp::Historical* pGlobalHistorical = nullptr)
{
  return _lookForAnActionToDoConst(pProblem, pDomain, pNow, pGlobalHistorical).actionInstance.toStr();
}

cp::OneStepOfPlannerResult _lookForAnActionToDoThenNotify(
    cp::Problem& pProblem,
    const cp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  auto res = cp::lookForAnActionToDo(pProblem, pDomain, pNow);
  if (res)
  {
    auto itAction = pDomain.actions().find(res->actionInstance.actionId);
    if (itAction != pDomain.actions().end())
      pProblem.notifyActionDone(*res, itAction->second.effect.factsModifications, pNow,
                                &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);
    return *res;
  }
  return cp::OneStepOfPlannerResult("", {}, cp::Goal("noGoal"), 0);
}

void _setGoalsForAPriority(cp::Problem& pProblem,
                           const std::vector<cp::Goal>& pGoals,
                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                           int pPriority = cp::Problem::defaultPriority)
{
  pProblem.setGoals(pGoals, pNow, pPriority);
}


void _test_createEmptyGoal()
{
  cp::Goal("goal_name", -1, "");
}

void _test_goalToStr()
{
  assert_eq<std::string>("imply(condition, goal_name)", cp::Goal("imply(condition, goal_name)").toStr());
  assert_eq<std::string>("persist(imply(condition, goal_name))", cp::Goal("persist(imply(condition, goal_name))").toStr());
  assert_eq<std::string>("oneStepTowards(goal_name)", cp::Goal("oneStepTowards(goal_name)").toStr());
}

void _test_setOfFactsFromStr()
{
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr("a,b", ',');
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    assert(sOfFacts.facts.count(cp::Fact("b")) == 1);
  }
  const std::vector<char> spearators = {',', '&'};
  for (auto currSeparator : spearators)
  {
    std::string currSeparatorStr(1, currSeparator);
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr(" a" + currSeparatorStr + " !b=ok " + currSeparatorStr + " c(r)=t " + currSeparatorStr + " d(!b=ok, c(r)=t)=val " + currSeparatorStr + " e ", currSeparator);
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    cp::FactOptional bFact("b");
    bFact.isFactNegated = true;
    bFact.fact.name = "b";
    bFact.fact.value = "ok";
    assert(sOfFacts.notFacts.count(bFact.fact) == 1);
    cp::FactOptional cFact("c");
    cFact.fact.parameters.emplace_back(cp::FactOptional("r"));
    cFact.fact.value = "t";
    assert(sOfFacts.facts.count(cFact.fact) == 1);
    cp::FactOptional dFact("d");
    dFact.fact.parameters.emplace_back(bFact);
    dFact.fact.parameters.emplace_back(cFact);
    dFact.fact.value = "val";
    assert(sOfFacts.facts.count(dFact.fact) == 1);
    assert(sOfFacts.facts.count(cp::Fact("e")) == 1);
    assert(sOfFacts.facts.count(cp::Fact("f")) == 0);
  }
}


void _automaticallyRemoveGoalsWithAMaxTimeToKeepInactiveEqualTo0()
{
  cp::Problem problem;
  assert_eq<std::size_t>(0u, problem.goals().size());
  problem.pushBackGoal(cp::Goal(_fact_advertised), {}, 10);
  problem.pushBackGoal(cp::Goal(_fact_beHappy), {}, 9);
  assert_eq<std::size_t>(2u, problem.goals().size());
  problem.pushBackGoal(cp::Goal(_fact_checkedIn, 0), {}, 9);
  assert_eq<std::size_t>(2u, problem.goals().size());
  assert_eq<std::size_t>(1u, problem.goals().find(9)->second.size());
}


void _maxTimeToKeepInactiveEqualTo0UnderAnAlreadySatisfiedGoal()
{
  cp::Problem problem;
  assert_eq<std::size_t>(0u, problem.goals().size());
  problem.pushBackGoal(cp::Goal("persist(!" + _fact_a + ")"), {}, 10);
  assert_eq<std::size_t>(1u, problem.goals().size());
  problem.pushBackGoal(cp::Goal(_fact_checkedIn, 0), {}, 9);
  assert_eq<std::size_t>(2u, problem.goals().size());
}

void _noPreconditionGoalImmediatlyReached()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain));
  assert_true(!problem.goals().empty());
  assert_true(!problem.hasFact(_fact_beHappy));
  problem.addFact(_fact_beHappy, now);
  assert_true(problem.hasFact(_fact_beHappy));
}


void _removeGoalWhenItIsSatisfiedByAnAction()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});

  auto plannerResult = _lookForAnActionToDoThenNotify(problem, domain);
  assert_eq(_action_goodBoy, plannerResult.actionInstance.toStr());
  assert_eq(_fact_beHappy, plannerResult.fromGoal.toStr());
  assert_eq(10, plannerResult.fromGoalPriority);
  assert_true(problem.goals().empty());
}


void _removeAnAction()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  cp::GoalsRemovedTracker goalsRemovedTracker(problem);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  problem.pushFrontGoal(cp::Goal(_fact_checkedIn, -1, goalGroupId), {});
  problem.pushFrontGoal(cp::Goal(_fact_greeted, -1, goalGroupId), {});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq<std::size_t>(0, goalsRemoved.size());
  problem.removeGoals(goalGroupId, {});
  assert_eq<std::size_t>(2, goalsRemoved.size());
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_goodBoy, _lookForAnActionToDoConstStr(problem, domain));
  problem.clearGoals({});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _notifyGoalRemovedWhenItIsImmediatlyRemoved()
{
  cp::Problem problem;

  cp::GoalsRemovedTracker goalsRemovedTracker(problem);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });

  problem.addGoals({{10, {_fact_a}}}, {});

  problem.addGoals({{9, {cp::Goal(_fact_b, 0)}}}, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.pushBackGoal(cp::Goal(_fact_c, 0), {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_c, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.pushFrontGoal(cp::Goal(_fact_d, 0), {}, 9);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_d, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.pushFrontGoal(cp::Goal(_fact_e, 0), {}, 11);
  assert_eq<std::size_t>(0u, goalsRemoved.size());
  problem.changeGoalPriority(_fact_e, 9, true, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_e, *goalsRemoved.begin());
  goalsRemoved.clear();

  problem.setGoals({{10, {cp::Goal(_fact_a, 0)}}, {9, {cp::Goal(_fact_b, 0)}}}, {});
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_b, *goalsRemoved.begin());
}



void _handlePreconditionWithNegatedFacts()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(cp::FactCondition::fromStr("!" + _fact_checkedIn), {_fact_greeted}));
  actions.emplace(_action_joke, cp::Action(cp::FactCondition::fromStr("!" + _fact_checkedIn), {_fact_userSatisfied}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_userSatisfied), {_fact_checkedIn}));
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
  actions.emplace(action1, cp::Action(cp::FactCondition::fromStr(_fact_e + " & !" + _fact_b), cp::SetOfFacts({}, {_fact_c})));
  actions.emplace(action2, cp::Action({}, cp::SetOfFacts({}, {_fact_b})));
  actions.emplace(action3, cp::Action(cp::FactCondition::fromStr(_fact_a + " & !" + _fact_c), {_fact_d}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setFacts({_fact_a, _fact_b, _fact_c}, now);
  _setGoalsForAPriority(problem, {_fact_d});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
  problem.addFact(_fact_e, now);
  assert_eq(action2, _lookForAnActionToDoStr(problem, domain));
}

void _noPlanWithALengthOf2()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted), {_fact_beHappy}));
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action(cp::FactCondition::fromStr(_fact_greeted), {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_checkedIn), {_fact_beHappy}));
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_checkedIn), {_fact_beHappy}));
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_checkedIn), {_fact_beHappy}));
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_checkedIn), {_fact_beHappy}));
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
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_checkedIn), {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _privigelizeTheActionsThatHaveManyPreconditions()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithQrCode, cp::Action(cp::FactCondition::fromStr(_fact_hasQrCode), {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action(cp::FactCondition::fromStr(_fact_hasCheckInPasword), {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_checkedIn), {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode}, now);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword}, now);
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}

void _preconditionThatCannotBeSolved()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action(cp::FactCondition::fromStr(_fact_hasQrCode), {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action(cp::FactCondition::fromStr(_fact_hasCheckInPasword), {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_checkedIn), {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _preferInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({}, {_fact_checkedIn}, {_fact_hasQrCode}));
  actions.emplace(_action_checkInWithPassword, cp::Action({}, {_fact_checkedIn}, {_fact_hasCheckInPasword}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_greeted + " & " + _fact_checkedIn), {_fact_beHappy}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode}, {});
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword}, now);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  problem.setFacts({}, now);
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode}, now);
  assert_eq(_action_checkInWithQrCode + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword}, now);
  assert_eq(_action_checkInWithPassword + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  auto dontHaveAQrCode = cp::SetOfFacts::fromStr("!" + _fact_hasQrCode, ',');
  actions.emplace(_action_checkInWithRealPerson, cp::Action({}, {_fact_checkedIn}, dontHaveAQrCode));
  problem.setFacts({}, now);
  assert_eq(_action_checkInWithRealPerson + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}


void _preferWhenPreconditionAreCloserToTheRealFacts()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_presented + "&" + _fact_checkedIn), {_fact_beHappy}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_beginOfConversation}, now);
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _avoidToDo2TimesTheSameActionIfPossble()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}, {_fact_beginOfConversation}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_presented + "&" + _fact_checkedIn), {_fact_beHappy}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _solveStr(problem, actions));

  problem.setFacts({}, now);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_presentation + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _takeHistoricalIntoAccount()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted, _fact_presented}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_presented + "&" + _fact_checkedIn), {_fact_beHappy}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));

  assert_eq(_action_presentation + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions, &problem.historical));
}


void _goDoTheActionThatHaveTheMostPrerequisitValidated()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_advertise, cp::Action({}, {_fact_advertised}));
  actions.emplace(_action_checkIn, cp::Action(cp::FactCondition::fromStr(_fact_is_close), {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_advertised + "&" + _fact_checkedIn), {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setFacts({_fact_is_close}, now);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoStr(problem, domain));
}


void _checkNotInAPrecondition()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(cp::FactCondition::fromStr("!" + _fact_checkedIn), {_fact_greeted}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConstStr(problem, domain));
  problem.modifyFacts({_fact_checkedIn}, now);
  assert_eq(std::string(), _lookForAnActionToDoConstStr(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::Domain domain(std::move(actions));
  cp::Problem problem;
  problem.setFacts({_fact_greeted}, now);
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq<std::size_t>(1, problem.goals().size());
  _lookForAnActionToDo(problem, domain);
  assert_eq<std::size_t>(0, problem.goals().size());
}


void _checkActionHasAFact()
{
  cp::WorldModification effect(cp::SetOfFacts::fromStr(_fact_a + " & !" + _fact_b, '&'));
  effect.potentialFactsModifications.add({_fact_c});
  effect.goalsToAdd[cp::Problem::defaultPriority] = {_fact_d};
  const cp::Action action(cp::FactCondition::fromStr(_fact_e), effect, {_fact_f});
  assert_true(action.hasFact(_fact_a));
  assert_true(action.hasFact(_fact_b));
  assert_true(action.hasFact(_fact_c));
  assert_true(action.hasFact(_fact_d));
  assert_true(action.hasFact(_fact_e));
  assert_true(action.hasFact(_fact_f));
  assert_false(action.hasFact(_fact_g));
}

void _checkReplacefactInAnExpression()
{
  char sep = '\n';
  auto setOfFacts = cp::SetOfFacts::fromStr("++${var-name}", sep);
  setOfFacts.replaceFact(cp::Fact::fromStr("var-name"), cp::Fact::fromStr("var-new-name"));
  assert_eq<std::string>("++${var-new-name}", setOfFacts.toStr("\n"));
  setOfFacts = cp::SetOfFacts::fromStr("${var-name-to-check}=10", sep);
  setOfFacts.replaceFact(cp::Fact::fromStr("var-name-to-check"), cp::Fact::fromStr("var-name-to-check-new"));
  assert_eq<std::string>("${var-name-to-check-new}=10", setOfFacts.toStr("\n"));
}

void _checkActionReplacefact()
{
  cp::WorldModification effect(cp::SetOfFacts::fromStr(_fact_a + " & !" + _fact_b, '&'));
  effect.potentialFactsModifications.add({_fact_c});
  effect.goalsToAdd[cp::Problem::defaultPriority] = {_fact_d};
  cp::Action action(cp::FactCondition::fromStr(_fact_e), effect, {_fact_f});
  assert_true(action.hasFact(_fact_a));
  assert_false(action.hasFact(_fact_g));
  action.replaceFact(_fact_a, _fact_g);
  assert_false(action.hasFact(_fact_a));
  assert_true(action.hasFact(_fact_g));
}



void _testIncrementOfVariables()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  const cp::Action actionQ1({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n'));
  const cp::Action actionFinishToActActions(cp::FactCondition::fromStr("${number-of-question}=${max-number-of-questions}"), {_fact_askAllTheQuestions});
  const cp::Action actionSayQuestionBilan(cp::FactCondition::fromStr(_fact_askAllTheQuestions), {_fact_finishToAskQuestions});
  actions.emplace(_action_askQuestion1, actionQ1);
  actions.emplace(_action_askQuestion2, cp::Action({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n')));
  actions.emplace(_action_finisehdToAskQuestions, actionFinishToActActions);
  actions.emplace(_action_sayQuestionBilan, actionSayQuestionBilan);
  cp::Domain domain(std::move(actions));

  std::string initFactsStr = "${number-of-question}=0&${max-number-of-questions}=3";
  cp::Problem problem;
  problem.modifyFacts(cp::SetOfFacts::fromStr(initFactsStr, '&'), now);
  assert(problem.isConditionTrue(cp::FactCondition::fromStr(initFactsStr)));
  assert(problem.isConditionTrue(actionQ1.precondition));
  assert(!problem.isConditionTrue(actionFinishToActActions.precondition));
  assert(!problem.isConditionTrue(actionSayQuestionBilan.precondition));
  assert(problem.isConditionTrue(cp::FactCondition::fromStr("${max-number-of-questions}=${number-of-question}+3")));
  assert(!problem.isConditionTrue(cp::FactCondition::fromStr("${max-number-of-questions}=${number-of-question}+4")));
  assert(problem.isConditionTrue(cp::FactCondition::fromStr("${max-number-of-questions}=${number-of-question}+4-1")));
  for (std::size_t i = 0; i < 3; ++i)
  {
    _setGoalsForAPriority(problem, {_fact_finishToAskQuestions});
    auto actionToDo = _lookForAnActionToDoStr(problem, domain);
    if (i == 0 || i == 2)
      assert_eq<std::string>(_action_askQuestion1, actionToDo);
    else
      assert_eq<std::string>(_action_askQuestion2, actionToDo);
    problem.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    problem.modifyFacts(itAction->second.effect.factsModifications, now);
    problem.modifyFacts(cp::SetOfFacts({}, {_fact_askAllTheQuestions}), now);
  }
  assert(problem.isConditionTrue(actionQ1.precondition));
  assert(problem.isConditionTrue(actionFinishToActActions.precondition));
  assert(!problem.isConditionTrue(actionSayQuestionBilan.precondition));
  _setGoalsForAPriority(problem, {_fact_finishToAskQuestions});
  auto actionToDo = _lookForAnActionToDoStr(problem, domain);
  assert_eq<std::string>(_action_finisehdToAskQuestions, actionToDo);
  problem.historical.notifyActionDone(actionToDo);
  auto itAction = domain.actions().find(actionToDo);
  assert(itAction != domain.actions().end());
  problem.modifyFacts(itAction->second.effect.factsModifications, now);
  assert_eq<std::string>(_action_sayQuestionBilan, _lookForAnActionToDoStr(problem, domain));
  assert(problem.isConditionTrue(actionQ1.precondition));
  assert(problem.isConditionTrue(actionFinishToActActions.precondition));
  assert(problem.isConditionTrue(actionSayQuestionBilan.precondition));
  problem.modifyFacts(actionSayQuestionBilan.effect.factsModifications, now);
}

void _precoditionEqualEffect()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr(_fact_beHappy), {_fact_beHappy}));
  cp::Domain domain(std::move(actions));
  assert_true(domain.actions().empty());

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDoStr(problem, domain).empty());
}


void _circularDependencies()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action(cp::FactCondition::fromStr(_fact_greeted), {_fact_checkedIn}));
  actions.emplace("check-in-pwd", cp::Action(cp::FactCondition::fromStr(_fact_hasCheckInPasword), {_fact_checkedIn}));
  actions.emplace("inverse-of-check-in-pwd", cp::Action(cp::FactCondition::fromStr(_fact_checkedIn), {_fact_hasCheckInPasword}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConstStr(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action(cp::FactCondition::fromStr(_fact_beSad), cp::WorldModification({}, {_fact_beSad})));
  actions.emplace(_action_goodBoy, cp::Action(cp::FactCondition::fromStr("!" + _fact_beSad), {_fact_beHappy}));

  cp::Historical historical;
  cp::Problem problem;
  problem.addFact(_fact_beSad, now);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_joke + _sep +
            _action_goodBoy, _solveStr(problem, actions, {}, &historical));
}


void _actionWithConstantValue()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, {cp::Fact::fromStr("place=kitchen")});
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("place=kitchen")});
  assert_eq(_action_navigate, _solveStr(problem, actions));
}


void _actionWithParameterizedValue()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigate({}, {cp::Fact::fromStr("place=target")});
  navigate.parameters.emplace_back("target");
  actions.emplace(_action_navigate, navigate);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("place=kitchen")});
  assert_eq(_action_navigate + "(target -> kitchen)", _solveStr(problem, actions));
}


void _actionWithParameterizedParameter()
{
  std::map<std::string, cp::Action> actions;
  cp::Action joke({}, {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("isHappy(1)")});
  assert_eq(_action_joke + "(human -> 1)", _solveStr(problem, actions));
}



void _actionWithParametersInPreconditionsAndEffects()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::Action joke(cp::FactCondition::fromStr("isEngaged(human)"), {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  problem.addFact(cp::Fact::fromStr("isEngaged(1)"), now);
  _setGoalsForAPriority(problem, {cp::Goal("isHappy(1)")});
  assert_eq(_action_joke + "(human -> 1)", _solveStr(problem, actions));
}


void _actionWithParametersInPreconditionsAndEffectsWithoutSolution()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::Action joke(cp::FactCondition::fromStr("isEngaged(human)"), {cp::Fact::fromStr("isHappy(human)")});
  joke.parameters.emplace_back("human");
  actions.emplace(_action_joke, joke);

  cp::Problem problem;
  problem.addFact(cp::Fact::fromStr("isEngaged(2)"), now);
  _setGoalsForAPriority(problem, {cp::Goal("isHappy(1)")});
  assert_eq<std::string>("", _solveStr(problem, actions));
}

void _actionWithParametersInsideThePath()
{
  std::map<std::string, cp::Action> actions;
  cp::Action navigateAction({}, {cp::Fact::fromStr("place=target")});
  navigateAction.parameters.emplace_back("target");
  actions.emplace(_action_navigate, navigateAction);

  actions.emplace(_action_welcome, cp::Action(cp::FactCondition::fromStr("place=entrance"), {cp::Fact::fromStr("welcomePeople")}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("welcomePeople")});
  assert_eq<std::string>(_action_navigate + "(target -> entrance)" + _sep +
                         _action_welcome, _solveStr(problem, actions));
}


void _testPersistGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_welcome, cp::Action({}, {cp::Fact::fromStr("welcomePeople")}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("welcomePeople")});
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(0, problem.goals().size());

  problem = cp::Problem();
  _setGoalsForAPriority(problem, {cp::Goal("persist(welcomePeople)")});
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>(_action_welcome, _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goals().size());
  assert_eq<std::string>("", _solveStr(problem, actions));
  assert_eq<std::size_t>(1, problem.goals().size());
}


void _testPersistImplyGoal()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("persist(imply(" + _fact_greeted + ", " + _fact_checkedIn + "))")});
  assert_eq<std::string>("", _solveStr(problem, actions));
  problem.addFact(_fact_greeted, now);
  assert_eq<std::string>(_action_checkIn, _solveStr(problem, actions));
}


void _testImplyGoal()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  cp::Problem problem;
  _setGoalsForAPriority(problem, {cp::Goal("imply(" + _fact_greeted + ", " + _fact_checkedIn + ")")});
  assert_eq<std::string>("", _solveStr(problem, actions));
  // It is not a persistent goal it is removed
  problem.addFact(_fact_greeted, now);
  assert_eq<std::string>("", _solveStr(problem, actions));
}


void _checkPreviousBugAboutSelectingAnInappropriateAction()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  auto removeLearntBehavior = cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior, ',');
  actions.emplace(_action_askQuestion1, cp::Action(cp::FactCondition::fromStr(_fact_engagedWithUser), {_fact_userSatisfied}, removeLearntBehavior));
  actions.emplace(_action_checkIn, cp::Action({}, cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior + " & " + _fact_advertised, '&')));
  cp::Domain domain(std::move(actions));

  cp::Historical historical;
  cp::Problem problem;
  problem.setFacts({_fact_engagedWithUser}, now);
  _setGoalsForAPriority(problem, {"persist(" + _fact_userSatisfied + ")"});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
  problem.removeFact(_fact_userSatisfied, now);
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _dontLinkActionWithPreferredInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  auto removeLearntBehavior = cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior, ',');
  actions.emplace(_action_askQuestion1, cp::Action({}, {_fact_userSatisfied}, {_fact_checkedIn}));
  actions.emplace(_action_checkIn, cp::Action(cp::FactCondition::fromStr(_fact_engagedWithUser), {_fact_checkedIn}));
  cp::Domain domain(std::move(actions));

  cp::Historical historical;
  cp::Problem problem;
  problem.setFacts({_fact_engagedWithUser}, now);
  _setGoalsForAPriority(problem, {_fact_userSatisfied});
  assert_eq<std::string>(_action_askQuestion1, _solveStr(problem, actions));
}


void _checkPriorities()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setGoals({{10, {_fact_greeted}}, {9, {_fact_beHappy}}}, {});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _stackablePropertyOfGoals()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setGoals({{10, {cp::Goal(_fact_greeted, 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  problem2.setGoals({{10, {cp::Goal(_fact_greeted, 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  problem2.pushFrontGoal(_fact_presented, {}, 10);
  assert_eq(_action_presentation + _sep +
            _action_goodBoy, _solveStr(problem2, actions));
}



void _doNotRemoveAGoalWithMaxTimeToKeepInactiveEqual0BelowAGoalWithACondotionNotSatisfied()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  actions.emplace(_action_presentation, cp::Action({}, {_fact_presented}));
  cp::Domain domain(std::move(actions));

  // Even if _fact_checkedIn has maxTimeToKeepInactive equal to 0, it is not removed because the goal with a higher priority is inactive.
  cp::Problem problem;
  problem.setGoals({{10, {cp::Goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));

  cp::Problem problem2;
  problem2.addFact(_fact_presented, now); // The difference here is that the condition of the first goal is satisfied
  problem2.setGoals({{10, {cp::Goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem2, actions));


  cp::Problem problem3;
  problem3.setGoals({{10, {cp::Goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  problem3.addFact(_fact_presented, now); // The difference here is that the condition is validated after the add of the goal
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem3, actions));


  cp::Problem problem4;
  problem4.setGoals({{10, {cp::Goal("imply(" + _fact_presented + ", " + _fact_greeted + ")", 0)}}, {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  problem4.addFact(_fact_presented, now); // Here _fact_checkedIn goal shoud be removed from the stack
  problem4.removeFact(_fact_presented, now); // The difference here is that the condition was validated only punctually
  assert_eq(_action_goodBoy, _solveStr(problem4, actions));
}



void _checkMaxTimeToKeepInactiveForGoals()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));


  cp::Problem problem;
  problem.setGoals({{10, {_fact_greeted, cp::Goal(_fact_checkedIn, 60)}}}, now);
  assert_eq(_action_greet + _sep +
            _action_checkIn, _solveStr(problem, actions, now));


  cp::Problem problem2;
  problem2.setGoals({{10, {_fact_greeted, cp::Goal(_fact_checkedIn, 60)}}}, now);
  now = std::make_unique<std::chrono::steady_clock::time_point>(*now + std::chrono::seconds(100));
  assert_eq(_action_greet, _solveStr(problem2, actions, now));
}



void _changePriorityOfGoal()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  std::map<int, std::vector<cp::Goal>> goalsFromSubscription;
  cp::Problem problem;
  auto onGoalsChangedConnection = problem.onGoalsChanged.connectUnsafe([&](const std::map<int, std::vector<cp::Goal>>& pGoals) {
    goalsFromSubscription = pGoals;
  });
  cp::GoalsRemovedTracker goalsRemovedTracker(problem);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });

  problem.setGoals({{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, now);
  {
    auto& goals = problem.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(1, goals.find(9)->second.size());
    assert_eq<std::size_t>(2, goals.find(10)->second.size());
  }

  problem.changeGoalPriority(_fact_checkedIn, 9, true, now);
  {
    auto& goals = problem.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(2, goals.find(9)->second.size());
    assert_eq(_fact_checkedIn, goals.find(9)->second[0].toStr());
    assert_eq(_fact_userSatisfied, goals.find(9)->second[1].toStr());
    assert_eq<std::size_t>(1, goals.find(10)->second.size());
  }

  problem.setGoals({{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, now);
  problem.changeGoalPriority(_fact_checkedIn, 9, false, now);
  {
    auto& goals = problem.goals();
    assert_eq(goalsFromSubscription, goals);
    assert_true(goalsRemoved.empty());
    assert_eq<std::size_t>(2, goals.find(9)->second.size());
    assert_eq(_fact_userSatisfied, goals.find(9)->second[0].toStr());
    assert_eq(_fact_checkedIn, goals.find(9)->second[1].toStr());
    assert_eq<std::size_t>(1, goals.find(10)->second.size());
  }

  problem.setGoals({{10, {_fact_greeted, _fact_checkedIn}}}, now);
  problem.changeGoalPriority(_fact_checkedIn, 9, true, now);
  {
    auto& goals = problem.goals();
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
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn, _fact_punctual_p1}));
  cp::Domain domain(std::move(actions));

  std::set<cp::Fact> factsChangedFromSubscription;
  cp::Problem problem;
  problem.addFact(_fact_beginOfConversation, now);
  auto factsChangedConnection = problem.onFactsChanged.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsChangedFromSubscription = pFacts;
  });
  std::list<cp::Fact> punctualFactsAdded;
  auto onPunctualFactsConnection = problem.onPunctualFacts.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    punctualFactsAdded.insert(punctualFactsAdded.end(), pFacts.begin(), pFacts.end());
  });
  std::list<cp::Fact> factsAdded;
  auto onFactsAddedConnection = problem.onFactsAdded.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsAdded.insert(factsAdded.end(), pFacts.begin(), pFacts.end());
  });
  std::list<cp::Fact> factsRemoved;
  auto onFactsRemovedConnection = problem.onFactsRemoved.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsRemoved.insert(factsRemoved.end(), pFacts.begin(), pFacts.end());
  });

  problem.setGoals({{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, now);
  assert_eq({}, factsChangedFromSubscription);

  auto plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_greet, plannerResult.actionInstance.actionId);
  assert_eq({_fact_beginOfConversation, _fact_greeted}, factsChangedFromSubscription);
  assert_eq({}, punctualFactsAdded);
  assert_eq({_fact_greeted}, factsAdded);
  factsAdded.clear();
  assert_eq({}, factsRemoved);

  plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_checkIn, plannerResult.actionInstance.actionId);
  assert_eq({_fact_beginOfConversation, _fact_greeted, _fact_checkedIn}, factsChangedFromSubscription);
  assert_eq({_fact_punctual_p1}, punctualFactsAdded);
  assert_eq({_fact_checkedIn}, factsAdded);
  assert_eq({}, factsRemoved);
  problem.removeFact(_fact_greeted, now);
  assert_eq({_fact_beginOfConversation, _fact_checkedIn}, factsChangedFromSubscription);
  assert_eq({_fact_punctual_p1}, punctualFactsAdded);
  assert_eq({_fact_checkedIn}, factsAdded);
  assert_eq({_fact_greeted}, factsRemoved);

  onFactsRemovedConnection.disconnect();
  onPunctualFactsConnection.disconnect();
  onFactsAddedConnection.disconnect();
  factsChangedConnection.disconnect();
}




void _checkInferences()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  assert_eq<std::string>("", _solveStr(problem, actions, now));
  // Inference: if (_fact_headTouched) then remove(_fact_headTouched) and addGoal(_fact_checkedIn)
  setOfInferences->addInference("inference1",
                                cp::Inference(cp::FactCondition::fromStr(_fact_headTouched),
                                              cp::SetOfFacts({}, {_fact_headTouched}),
                                              {{{9, {_fact_checkedIn}}}}));
  assert_eq<std::string>("", _solveStr(problem, actions, now));
  problem.addFact(_fact_headTouched, now);
  assert_true(!problem.hasFact(_fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, actions, now));
}



void _checkInferencesWithImply()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal("persist(imply(" + _fact_userWantsToCheckedIn + ", " + _fact_checkedIn + "))")});
  // Inference: if (_fact_headTouched) then add(_fact_userWantsToCheckedIn) and remove(_fact_headTouched)
  setOfInferences->addInference("inference1",
                                cp::Inference(cp::FactCondition::fromStr(_fact_headTouched),
                                              cp::SetOfFacts({_fact_userWantsToCheckedIn}, {_fact_headTouched})));
  assert_eq<std::string>("", _solveStr(problem, actions, now));
  problem.addFact(_fact_headTouched, now);
  assert_true(!problem.hasFact(_fact_headTouched)); // removed because of the inference
  assert_eq(_action_checkIn, _solveStr(problem, actions, now));
}


void _checkInferenceWithPunctualCondition()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, cp::WorldModification({}, {_fact_userWantsToCheckedIn})));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  // Inference: if (_fact_punctual_headTouched) then add(_fact_userWantsToCheckedIn)
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_headTouched),
                                                            {_fact_userWantsToCheckedIn}));
  assert_eq<std::string>("", _solveStr(problem, actions, now));
  problem.addFact(_fact_punctual_headTouched, now);
  assert_true(!problem.hasFact(_fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, actions, now));
}


void _checkInferenceAtEndOfAPlan()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, cp::WorldModification({ _fact_punctual_checkedIn })));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_headTouched),
                                                            {_fact_userWantsToCheckedIn}));
  setOfInferences->addInference("inference2", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_checkedIn),
                                                            cp::SetOfFacts::fromStr("!" + _fact_userWantsToCheckedIn, '&')));
  assert_eq<std::string>("", _solveStr(problem, actions, now));
  problem.addFact(_fact_punctual_headTouched, now);
  assert_true(!problem.hasFact(_fact_punctual_headTouched)); // because it is a punctual fact
  assert_eq(_action_checkIn, _solveStr(problem, actions, now));
}


void _checkInferenceInsideAPlan()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, cp::WorldModification({ _fact_a })));
  actions.emplace(action2, cp::Action(cp::FactCondition::fromStr(_fact_c), cp::WorldModification({ _fact_d })));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal(_fact_d)});
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_a),
                                                            {_fact_b}));
  setOfInferences->addInference("inference2", cp::Inference(cp::FactCondition::fromStr(_fact_b + "&" + _fact_d),
                                                            {_fact_c}));
  assert_eq<std::string>("", _solveStrConst(problem, actions));
  setOfInferences->addInference("inference3", cp::Inference(cp::FactCondition::fromStr(_fact_b),
                                                            {_fact_c}));
  assert_eq(action1 + _sep + action2, _solveStrConst(problem, actions)); // check with a copy of the problem
  assert_true(!problem.hasFact(_fact_a));
  assert_true(!problem.hasFact(_fact_b));
  assert_true(!problem.hasFact(_fact_c));
  assert_true(!problem.hasFact(_fact_d));
  assert_eq(action1 + _sep + action2, _solveStr(problem, actions));
  assert_true(problem.hasFact(_fact_a));
  assert_true(problem.hasFact(_fact_b));
  assert_true(problem.hasFact(_fact_c));
  assert_true(problem.hasFact(_fact_d));
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
  actions.emplace(action1, cp::Action({}, cp::WorldModification({ _fact_a })));
  actions.emplace(action2, cp::Action(cp::FactCondition::fromStr(_fact_c),
                                      cp::WorldModification({ _fact_d })));
  actions.emplace(action3, cp::Action(cp::FactCondition::fromStr(_fact_c + "&" + _fact_f),
                                      cp::WorldModification({ _fact_e })));
  actions.emplace(action4, cp::Action(cp::FactCondition::fromStr(_fact_b),
                                      cp::WorldModification({ _fact_f })));
  actions.emplace(action5, cp::Action(cp::FactCondition::fromStr(_fact_b),
                                      cp::WorldModification({ _fact_g })));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal("imply(" + _fact_g + ", " + _fact_d)});
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_a),
                                                            { _fact_b },
                                                            {{cp::Problem::defaultPriority, {_fact_e}}}));
  assert_eq<std::string>("", _solveStrConst(problem, actions));
  setOfInferences->addInference("inference2", cp::Inference(cp::FactCondition::fromStr(_fact_b),
                                                            { _fact_c }));
  assert_eq<std::string>("", _solveStrConst(problem, actions));
  problem.addFact(_fact_g, now);
  assert_eq(action1 + _sep + action4 + _sep + action3 + _sep + action2, _solveStr(problem, actions));
}

void _checkThatUnReachableCannotTriggeranInference()
{
  const std::string action1 = "action1";
  const std::string inference1 = "inference1";
  const std::string soi = "soi";

  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, {_fact_unreachable_u1}));
  cp::Domain domain(std::move(actions));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_a});
  problem.addSetOfInferences(soi, setOfInferences);
  setOfInferences->addInference(inference1, cp::Inference(cp::FactCondition::fromStr(_fact_unreachable_u1),
                                                          { _fact_a }));
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
  assert_true(!problem.hasFact(_fact_unreachable_u1));
  assert_true(!problem.hasFact(_fact_a));
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
  // check inferences removal
  problem.removeSetOfInferences(soi);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
  _setGoalsForAPriority(problem, {_fact_a});
  problem.addSetOfInferences(soi, setOfInferences);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
  setOfInferences->removeInference(inference1);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
}



void _testQuiz()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::WorldModification questionEffect(cp::SetOfFacts::fromStr("\n++${number-of-question}", '\n'));
  questionEffect.potentialFactsModifications = cp::SetOfFacts::fromStr(_fact_askAllTheQuestions, '\n');
  const cp::Action actionQ1({}, questionEffect);
  const cp::Action actionSayQuestionBilan(cp::FactCondition::fromStr(_fact_askAllTheQuestions),
                                          {_fact_finishToAskQuestions});
  actions.emplace(_action_askQuestion1, actionQ1);
  actions.emplace(_action_askQuestion2, cp::Action({}, questionEffect));
  actions.emplace(_action_sayQuestionBilan, actionSayQuestionBilan);
  cp::Domain domain(std::move(actions));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  const cp::Inference inferenceFinishToActActions(cp::FactCondition::fromStr("${number-of-question}=${max-number-of-questions}"), {_fact_askAllTheQuestions});
  setOfInferences->addInference(_action_finisehdToAskQuestions, inferenceFinishToActActions);

  auto initFacts = cp::SetOfFacts::fromStr("${number-of-question}=0\n${max-number-of-questions}=3", '\n');

  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {_fact_finishToAskQuestions});
  problem.modifyFacts(initFacts, now);
  for (std::size_t i = 0; i < 3; ++i)
  {
    auto actionToDo = _lookForAnActionToDoStr(problem, domain);
    if (i == 0 || i == 2)
      assert_eq<std::string>(_action_askQuestion1, actionToDo);
    else
      assert_eq<std::string>(_action_askQuestion2, actionToDo);
    problem.historical.notifyActionDone(actionToDo);
    auto itAction = domain.actions().find(actionToDo);
    assert(itAction != domain.actions().end());
    problem.modifyFacts(itAction->second.effect.factsModifications, now);
  }

  auto actionToDo = _lookForAnActionToDoStr(problem, domain);
  assert_eq(_action_sayQuestionBilan, actionToDo);
}


void _testGetNotSatisfiedGoals()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::Domain domain(std::move(actions));

  auto goal1 = "persist(!" + _fact_a + ")";
  auto goal2 = "persist(" + _fact_b + ")";
  auto goal3 = "imply(" + _fact_c + ", " + _fact_d + ")";
  auto goal4 = "persist(imply(!" + _fact_c + ", " + _fact_d + "))";

  cp::Problem problem;
  problem.addGoals({goal1}, now, cp::Problem::defaultPriority + 1);
  problem.addGoals({goal2, goal3, goal4}, {});

  assert_eq(goal1 + ", " + goal2 + ", " + goal3 + ", " + goal4, cp::printGoals(problem.goals()));
  assert_eq(goal2 + ", " + goal4, cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.addFact(_fact_a, {});
  assert_eq(goal1 + ", " + goal2 + ", " + goal4, cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.addFact(_fact_c, {});
  assert_eq(goal1 + ", " + goal2 + ", " + goal3, cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.addFact(_fact_d, {});
  assert_eq(goal1 + ", " + goal2, cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.removeFact(_fact_a, {});
  assert_eq(goal2, cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.addFact(_fact_b, {});
  assert_eq<std::string>("", cp::printGoals(problem.getNotSatisfiedGoals()));
  problem.removeFact(_fact_d, {});
  assert_eq(goal3, cp::printGoals(problem.getNotSatisfiedGoals()));
}



void _testGoalUnderPersist()
{
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  actions.emplace(action1, cp::Action({}, {_fact_b}));
  actions.emplace(action2, cp::Action({}, {_fact_c}));
  cp::Domain domain(std::move(actions));

  {
    cp::Problem problem;
    problem.addGoals({"persist(!" + _fact_a + ")"}, now, cp::Problem::defaultPriority + 2);
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.addGoals({"persist(!" + _fact_a + ")"}, now, cp::Problem::defaultPriority + 2);
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    problem.removeFirstGoalsThatAreAlreadySatisfied({});
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.addGoals({"persist(!" + _fact_a + ")"}, now, cp::Problem::defaultPriority + 2);
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    problem.addFact(_fact_a, now);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.addGoals({"persist(" + _fact_c + ")"}, now, cp::Problem::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
    assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain));
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.addGoals({"persist(" + _fact_c + ")"}, now, cp::Problem::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
    problem.removeFirstGoalsThatAreAlreadySatisfied({});
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.addGoals({_fact_c}, now, cp::Problem::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain).actionInstance.actionId);
    problem.removeFirstGoalsThatAreAlreadySatisfied({});
    problem.addGoals({cp::Goal(_fact_b, 0)}, now, cp::Problem::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain));
  }

  {
    cp::Problem problem;
    problem.pushBackGoal({"persist(!" + _fact_e + ")"}, now, cp::Problem::defaultPriority + 2);
    problem.pushBackGoal(_fact_c, now, cp::Problem::defaultPriority + 2);
    assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
    problem.removeFirstGoalsThatAreAlreadySatisfied(now);
    problem.addGoals({cp::Goal(_fact_b, 1)}, now, cp::Problem::defaultPriority);
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));
    assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));

    problem.removeFact(_fact_c, now);
    problem.pushBackGoal(_fact_c, now, cp::Problem::defaultPriority + 2);
    auto plannerResult = _lookForAnActionToDo(problem, domain, now);
    assert_eq(action2, plannerResult.actionInstance.actionId);

    now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now() + std::chrono::minutes(5));
    auto itAction = domain.actions().find(action2);
    if (itAction != domain.actions().end())
      problem.notifyActionDone(plannerResult, itAction->second.effect.factsModifications, now,
                                &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);

    problem.removeFirstGoalsThatAreAlreadySatisfied(now);
    assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId); // Not action1 because it was inactive for too long
  }

}


void _checkLinkedInferences()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());

  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, cp::WorldModification({ _fact_punctual_checkedIn })));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  _setGoalsForAPriority(problem, {cp::Goal("persist(!" + _fact_userWantsToCheckedIn + ")")});
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_p2),
                                                            {_fact_a}));
  setOfInferences->addInference("inference2", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_p5),
                                                            {_fact_punctual_p2, _fact_punctual_p3}));
  setOfInferences->addInference("inference3", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_p4),
                                                            {_fact_punctual_p5, _fact_punctual_p1}));

  assert_false(problem.hasFact(_fact_a));
  problem.addFact(_fact_punctual_p4, now);
  assert_true(problem.hasFact(_fact_a));
}



void _oneStepTowards()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<cp::ActionId, cp::Action> actions;
  cp::WorldModification greetWordModification;
  greetWordModification.potentialFactsModifications = cp::SetOfFacts::fromStr(_fact_greeted, '\n');
  actions.emplace(_action_greet, cp::Action({}, greetWordModification));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  static const std::string actionb = "actionb";
  actions.emplace(actionb, cp::Action({}, {_fact_b}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setGoals({{11, {cp::Goal("persist(imply(" + _fact_a + ", " + _fact_b + ")", 0)}},
                    {10, {cp::Goal("oneStepTowards(" + _fact_greeted + ")", 0)}},
                    {9, {cp::Goal(_fact_checkedIn, 0), _fact_beHappy}}}, {});
  problem.removeFirstGoalsThatAreAlreadySatisfied(now);
  assert_eq(_action_greet, _lookForAnActionToDoStr(problem, domain, now));
  assert_eq(_action_greet, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, now));
  assert_eq(_action_goodBoy, _lookForAnActionToDoStr(problem, domain, now));
  problem.addFact(_fact_a, now);
  assert_eq(actionb, _lookForAnActionToDoStr(problem, domain, now));
}


void _infrenceLinksFromManyInferencesSets()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";
  const std::string action2 = "action2";
  std::map<cp::ActionId, cp::Action> actions;
  cp::WorldModification actionWordModification;
  actionWordModification.potentialFactsModifications = cp::SetOfFacts::fromStr(_fact_d, '\n');
  actions.emplace(action1, cp::Action({}, actionWordModification));
  actions.emplace(action2, cp::Action({}, {_fact_c}));
  cp::Domain domain(std::move(actions));

  auto setOfInferences = std::make_shared<cp::SetOfInferences>();
  cp::Problem problem;
  problem.addSetOfInferences("soi", setOfInferences);
  assert_true(cp::Problem::defaultPriority >= 1);
  auto lowPriority = cp::Problem::defaultPriority - 1;
  setOfInferences->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_p2),
                                                            {}, {{lowPriority, {"oneStepTowards(" + _fact_d + ")"}}}));
  problem.setGoals({{lowPriority, {cp::Goal("oneStepTowards(" + _fact_d + ")", 0)}}}, {});

  auto setOfInferences2 = std::make_shared<cp::SetOfInferences>();
  problem.addSetOfInferences("soi2", setOfInferences2);
  setOfInferences2->addInference("inference1", cp::Inference(cp::FactCondition::fromStr(_fact_punctual_p1),
                                                             {_fact_b, _fact_punctual_p2}));
  setOfInferences2->addInference("inference2", cp::Inference(cp::FactCondition::fromStr(_fact_b),
                                                             {}, {{cp::Problem::defaultPriority, {"oneStepTowards(" + _fact_c + ")"}}}));

  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
  assert_eq<std::string>("", _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
  problem.addFact(_fact_punctual_p1, now);
  assert_eq(action2, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
}


void _factValueModification()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";

  std::map<cp::ActionId, cp::Action> actions;
  cp::WorldModification actionWordModification;
  actions.emplace(action1, cp::Action({}, cp::WorldModification({}, {_fact_b})));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  problem.setGoals({{10, {cp::Goal("persist(imply(" + _fact_a + "=a, " + "!" + _fact_b + ")", 0)}}}, {});

  problem.addFact(_fact_b, now);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, now));
  problem.addFact(_fact_a + "=a", now);
  assert_eq(action1, _lookForAnActionToDoStr(problem, domain, now));
  problem.addFact(_fact_a + "=b", now);
  assert_eq<std::string>("", _lookForAnActionToDoStr(problem, domain, now));
}


void _removeGoaWhenAnActionFinishesByAddingNewGoals()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  const std::string action1 = "action1";
  const std::string action2 = "action2";

  std::map<cp::ActionId, cp::Action> actions;
  cp::WorldModification actionWordModification;
  cp::WorldModification wm({_fact_a});
  wm.goalsToAddInCurrentPriority.push_back(cp::Goal(_fact_b, 0));
  actions.emplace(action1, cp::Action({}, wm));
  actions.emplace(action2, cp::Action({}, {_fact_b}));
  cp::Domain domain(std::move(actions));

  cp::Problem problem;
  cp::GoalsRemovedTracker goalsRemovedTracker(problem);
  std::set<std::string> goalsRemoved;
  auto onGoalsRemovedConnection = goalsRemovedTracker.onGoalsRemoved.connectUnsafe([&](const std::set<std::string>& pGoalsRemoved) {
    goalsRemoved = pGoalsRemoved;
  });
  problem.setGoals({{27, {_fact_a}}}, now);

  assert_eq(action1, _lookForAnActionToDoThenNotify(problem, domain, now).actionInstance.actionId);
  assert_eq<std::size_t>(1u, goalsRemoved.size());
  assert_eq(_fact_a, *goalsRemoved.begin());
  onGoalsRemovedConnection.disconnect();

  // The only remaning goal is the goal we just added
  assert_eq<std::size_t>(1u, problem.goals().size());
  assert_eq(27, problem.goals().begin()->first);
  assert_eq<std::size_t>(1u, problem.goals().begin()->second.size());
  assert_eq(_fact_b, problem.goals().begin()->second.begin()->toStr());
}


void _actionNavigationAndGrabObjectWithParameters()
{
  auto now = std::make_unique<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now());
  std::map<std::string, cp::Action> actions;
  cp::Action navAction({}, {cp::Fact::fromStr("location(me)=targetLocation")});
  navAction.parameters.emplace_back("targetLocation");
  actions.emplace(_action_navigate, navAction);

  cp::Action grabAction(cp::FactCondition::fromStr("equals(location(me), location(object))"),
                        {cp::Fact::fromStr("grab(me, object)")});
  grabAction.parameters.emplace_back("object");
  actions.emplace(_action_grab, grabAction);

  cp::Problem problem;
  problem.addFact(cp::Fact("location(me)=corridor"), now);
  problem.addFact(cp::Fact("location(object)=kitchen"), now);
  assert_eq<std::string>("kitchen", problem.getFactValue(cp::Fact("location(object)")));
  _setGoalsForAPriority(problem, {cp::Goal("grab(me, sweets)")});
  assert_eq<std::string>(_action_navigate + "(targetLocation -> kitchen), " + _action_grab + "(object -> sweets)", _solveStr(problem, actions));
}



}



int main(int argc, char *argv[])
{
  test_arithmeticEvaluator();
  planningDummyExample();
  planningExampleWithAPreconditionSolve();
  _test_createEmptyGoal();
  _test_goalToStr();
  _test_setOfFactsFromStr();
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
  _privigelizeTheActionsThatHaveManyPreconditions();
  _preconditionThatCannotBeSolved();
  _preferInContext();
  _preferWhenPreconditionAreCloserToTheRealFacts();
  _avoidToDo2TimesTheSameActionIfPossble();
  _takeHistoricalIntoAccount();
  _goDoTheActionThatHaveTheMostPrerequisitValidated();
  _checkNotInAPrecondition();
  _checkClearGoalsWhenItsAlreadySatisfied();
  _checkActionHasAFact();
  _checkReplacefactInAnExpression();
  _checkActionReplacefact();
  _testIncrementOfVariables();
  _precoditionEqualEffect();
  _circularDependencies();
  _triggerActionThatRemoveAFact();
  _actionWithConstantValue();
  _actionWithParameterizedValue();
  _actionWithParameterizedParameter();
  _actionWithParametersInPreconditionsAndEffects();
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
  _checkThatUnReachableCannotTriggeranInference();
  _testQuiz();
  _testGetNotSatisfiedGoals();
  _testGoalUnderPersist();
  _checkLinkedInferences();
  _oneStepTowards();
  _infrenceLinksFromManyInferencesSets();
  _factValueModification();
  _removeGoaWhenAnActionFinishesByAddingNewGoals();
  _actionNavigationAndGrabObjectWithParameters();

  std::cout << "chatbot planner is ok !!!!" << std::endl;
  return 0;
}
