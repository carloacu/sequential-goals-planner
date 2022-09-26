#include <contextualplanner/contextualplanner.hpp>
#include <contextualplanner/util/trackers/factschangedtracker.hpp>
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

const std::string _fact_advertised = "advertised";
const std::string _fact_beginOfConversation = "begin_of_conversation";
const std::string _fact_presented = "presented";
const std::string _fact_greeted = "greeted";
const std::string _fact_is_close = "is_close";
const std::string _fact_hasQrCode = "has_qrcode";
const std::string _fact_hasCheckInPasword = "has_check_in_password";
const std::string _fact_checkedIn = "checked_in";
const std::string _fact_beSad = "be_sad";
const std::string _fact_beHappy = "be_happy";
const std::string _fact_askAllTheQuestions = "ask_all_the_questions";
const std::string _fact_finishToAskQuestions = "finished_to_ask_questions";
const std::string _fact_engagedWithUser = "engaged_with_user";
const std::string _fact_userSatisfied = "user_satisfied";
const std::string _fact_robotLearntABehavior = "robot_learnt_a_behavior";

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


std::string _listOfStrToStr(const std::list<std::string>& pStrs)
{
  auto size = pStrs.size();
  if (size == 1)
    return pStrs.front();
  std::string res;
  bool firstIteration = true;
  for (const auto& currStr : pStrs)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += _sep;
    res += currStr;
  }
  return res;
}


std::string _solveStrConst(const cp::Problem& pProblem,
                           const std::map<std::string, cp::Action>& pActions,
                           cp::Historical* pGlobalHistorical = nullptr)
{
  auto problem = pProblem;
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::printResolutionPlan(problem, domain, {}, pGlobalHistorical));
}

std::string _solveStr(cp::Problem& pProblem,
                      const std::map<std::string, cp::Action>& pActions,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
                      cp::Historical* pGlobalHistorical = nullptr)
{
  cp::Domain domain(pActions);
  return _listOfStrToStr(cp::printResolutionPlan(pProblem, domain, pNow, pGlobalHistorical));
}

std::string _lookForAnActionToDo(
    std::map<std::string, std::string>& pParameters,
    cp::Problem& pProblem,
    const cp::Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {},
    const cp::Historical* pGlobalHistorical = nullptr)
{
  return cp::lookForAnActionToDo(pParameters, pProblem, pDomain, pNow, nullptr, nullptr, pGlobalHistorical);
}

std::string _lookForAnActionToDo(cp::Problem& pProblem,
                                 const cp::Domain& pDomain,
                                 const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow = {})
{
  std::map<std::string, std::string> parameters;
  auto actionId = cp::lookForAnActionToDo(parameters, pProblem, pDomain, pNow);
  return cp::printActionIdWithParameters(actionId, parameters);
}

std::string _lookForAnActionToDoConst(const cp::Problem& pProblem,
                                      const cp::Domain& pDomain)
{
  auto problem = pProblem;
  std::map<std::string, std::string> parameters;
  auto actionId = _lookForAnActionToDo(parameters, problem, pDomain);
  return cp::printActionIdWithParameters(actionId, parameters);
}


struct PlannerResult
{
  std::string actionId;
  std::map<std::string, std::string> parameters;
  const cp::Goal* goal = nullptr;
  int priority = 0;
};

PlannerResult _lookForAnActionToDoThenNotify(cp::Problem& pProblem,
                                             const cp::Domain& pDomain)
{
  PlannerResult res;
  res.actionId = cp::lookForAnActionToDo(res.parameters, pProblem, pDomain, {}, &res.goal, &res.priority);

  auto itAction = pDomain.actions().find(res.actionId);
  assert_true(itAction != pDomain.actions().end());
  pProblem.notifyActionDone(res.actionId, res.parameters, itAction->second.effect.factsModifications, {},
                            &itAction->second.effect.goalsToAdd);
  return res;
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
}

void _test_setOfFactsFromStr()
{
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr("a,b", ',');
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    assert(sOfFacts.facts.count(cp::Fact("b")) == 1);
  }
  {
    cp::SetOfFacts sOfFacts = cp::SetOfFacts::fromStr(" a, b=ok , c(r)=t , d(b=ok, c(r)=t)=val , e ", ',');
    assert(sOfFacts.facts.count(cp::Fact("a")) == 1);
    cp::Fact bFact;
    bFact.name = "b";
    bFact.value = "ok";
    assert(sOfFacts.facts.count(bFact) == 1);
    cp::Fact cFact;
    cFact.name = "c";
    cFact.parameters.emplace_back(cp::Fact("r"));
    cFact.value = "t";
    assert(sOfFacts.facts.count(cFact) == 1);
    cp::Fact dFact;
    dFact.name = "d";
    dFact.parameters.emplace_back(bFact);
    dFact.parameters.emplace_back(cFact);
    dFact.value = "val";
    assert(sOfFacts.facts.count(dFact) == 1);
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


void _noPreconditionGoalImmediatlyReached()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDo(problem, domain));
  assert_true(!problem.goals().empty());
  assert_true(!problem.hasFact(_fact_beHappy));
  problem.addFact(_fact_beHappy, now);
  assert_true(problem.hasFact(_fact_beHappy));
}


void _removeFirstGoalsThatAreAlreadySatisfied()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});

  auto plannerResult = _lookForAnActionToDoThenNotify(problem, domain);
  assert_eq(_action_goodBoy, cp::printActionIdWithParameters(plannerResult.actionId, plannerResult.parameters));
  assert_eq(_fact_beHappy, plannerResult.goal->toStr());
  assert_eq(10, plannerResult.priority);

  assert_true(!problem.goals().empty());
  problem.removeFirstGoalsThatAreAlreadySatisfied();
  assert_true(problem.goals().empty());
}


void _removeAnAction()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_goodBoy, _lookForAnActionToDoConst(problem, domain));
  domain.removeAction(_action_goodBoy);
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain));
}


void _removeSomeGoals()
{
  const std::string goalGroupId = "greetAndCheckIn";
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  problem.pushFrontGoal(cp::Goal(_fact_checkedIn, -1, goalGroupId), {});
  problem.pushFrontGoal(cp::Goal(_fact_greeted, -1, goalGroupId), {});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  problem.removeGoals(goalGroupId, {});
  assert_eq(_action_goodBoy, _lookForAnActionToDoConst(problem, domain));
}


void _noPlanWithALengthOf2()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _noPlanWithALengthOf3()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2preconditions()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_greet + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2Goals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}

void _2UnrelatedGoals()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _impossibleGoal()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _privigelizeTheActionsThatHaveManyPreconditions()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({_fact_hasQrCode}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted, _fact_beHappy});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasQrCode}, now);
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithQrCode + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));

  problem.setFacts({_fact_hasCheckInPasword}, now);
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  assert_eq(_action_greet + _sep +
            _action_checkInWithPassword + _sep +
            _action_goodBoy, _solveStrConst(problem, actions));
}

void _preconditionThatCannotBeSolved()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({_fact_hasQrCode}, {_fact_checkedIn}));
  actions.emplace(_action_checkInWithPassword, cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDo(problem, domain).empty());
}


void _preferInContext()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkInWithQrCode, cp::Action({}, {_fact_checkedIn}, {_fact_hasQrCode}));
  actions.emplace(_action_checkInWithPassword, cp::Action({}, {_fact_checkedIn}, {_fact_hasCheckInPasword}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

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
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

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
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

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
  actions.emplace(_action_goodBoy, cp::Action({_fact_presented, _fact_checkedIn}, {_fact_beHappy}));

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
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_advertised, _fact_checkedIn}, {_fact_beHappy}));
  cp::Domain domain(actions);

  cp::Problem problem;
  problem.setFacts({_fact_is_close}, now);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_checkIn, _lookForAnActionToDo(problem, domain));
}


void _checkShouldBeDoneAsap()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}, {}, true));
  actions.emplace(_action_checkIn, cp::Action({_fact_is_close}, {_fact_checkedIn}));
  actions.emplace(_action_goodBoy, cp::Action({_fact_greeted, _fact_checkedIn}, {_fact_beHappy}));

  cp::Problem problem;
  problem.setFacts({_fact_is_close}, now);
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq(_action_greet + _sep +
            _action_checkIn + _sep +
            _action_goodBoy, _solveStr(problem, actions));
}


void _checkNotInAPrecondition()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action(cp::SetOfFacts({}, {_fact_checkedIn}), {_fact_greeted}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq(_action_greet, _lookForAnActionToDoConst(problem, domain));
  problem.modifyFacts({_fact_checkedIn}, now);
  assert_eq(std::string(), _lookForAnActionToDoConst(problem, domain));
}


void _checkClearGoalsWhenItsAlreadySatisfied()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  cp::Domain domain(actions);
  cp::Problem problem;
  problem.setFacts({_fact_greeted}, now);
  _setGoalsForAPriority(problem, {_fact_greeted});
  assert_eq<std::size_t>(1, problem.goals().size());
  _lookForAnActionToDo(problem, domain);
  assert_eq<std::size_t>(0, problem.goals().size());
}


void _fromAndToStrOfSetOfFacts()
{
  char sep = '\n';
  auto setOfFacts = cp::SetOfFacts::fromStr("++${var-name}", sep);
  setOfFacts.rename(cp::Fact::fromStr("var-name"), cp::Fact::fromStr("var-new-name"));
  assert_eq<std::string>("++${var-new-name}", setOfFacts.toStr("\n"));
  setOfFacts = cp::SetOfFacts::fromStr("${var-name-to-check}=10", sep);
  setOfFacts.rename(cp::Fact::fromStr("var-name-to-check"), cp::Fact::fromStr("var-name-to-check-new"));
  assert_eq<std::string>("${var-name-to-check-new}=10", setOfFacts.toStr("\n"));
}


void _testIncrementOfVariables()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  const cp::Action actionQ1({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n'));
  const cp::Action actionFinishToActActions(cp::SetOfFacts::fromStr("${number-of-question}=${max-number-of-questions}", '\n'), {_fact_askAllTheQuestions}, {}, true);
  const cp::Action actionSayQuestionBilan({_fact_askAllTheQuestions}, {_fact_finishToAskQuestions});
  actions.emplace(_action_askQuestion1, actionQ1);
  actions.emplace(_action_askQuestion2, cp::Action({}, cp::SetOfFacts::fromStr(_fact_askAllTheQuestions + "\n++${number-of-question}", '\n')));
  actions.emplace(_action_finisehdToAskQuestions, actionFinishToActActions);
  actions.emplace(_action_sayQuestionBilan, actionSayQuestionBilan);
  cp::Domain domain(actions);

  std::string initFactsStr = "${number-of-question}=0\n${max-number-of-questions}=3";
  auto initFacts = cp::SetOfFacts::fromStr(initFactsStr, '\n');
  assert_eq(initFactsStr, initFacts.toStr("\n"));
  cp::Problem problem;
  problem.modifyFacts(initFacts, now);
  assert(cp::areFactsTrue(initFacts, problem));
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(!cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+3", '\n'), problem));
  assert(!cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4", '\n'), problem));
  assert(cp::areFactsTrue(cp::SetOfFacts::fromStr("${max-number-of-questions}=${number-of-question}+4-1", '\n'), problem));
  for (std::size_t i = 0; i < 3; ++i)
  {
    _setGoalsForAPriority(problem, {_fact_finishToAskQuestions});
    auto actionToDo = _lookForAnActionToDo(problem, domain);
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
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(!cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  _setGoalsForAPriority(problem, {_fact_finishToAskQuestions});
  auto actionToDo = _lookForAnActionToDo(problem, domain);
  assert_eq<std::string>(_action_finisehdToAskQuestions, actionToDo);
  problem.historical.notifyActionDone(actionToDo);
  auto itAction = domain.actions().find(actionToDo);
  assert(itAction != domain.actions().end());
  problem.modifyFacts(itAction->second.effect.factsModifications, now);
  assert_eq<std::string>(_action_sayQuestionBilan, _lookForAnActionToDo(problem, domain));
  assert(cp::areFactsTrue(actionQ1.preconditions, problem));
  assert(cp::areFactsTrue(actionFinishToActActions.preconditions, problem));
  assert(cp::areFactsTrue(actionSayQuestionBilan.preconditions, problem));
  problem.modifyFacts(actionSayQuestionBilan.effect.factsModifications, now);
}

void _precoditionEqualEffect()
{
  std::map<cp::ActionId, cp::Action> actions;
  actions.emplace(_action_goodBoy, cp::Action({_fact_beHappy}, {_fact_beHappy}));
  cp::Domain domain(actions);
  assert_true(domain.actions().empty());

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_true(_lookForAnActionToDo(problem, domain).empty());
}


void _circularDependencies()
{
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_greet, cp::Action({}, {_fact_greeted}));
  actions.emplace(_action_checkIn, cp::Action({_fact_greeted}, {_fact_checkedIn}));
  actions.emplace("check-in-pwd", cp::Action({_fact_hasCheckInPasword}, {_fact_checkedIn}));
  actions.emplace("inverse-of-check-in-pwd", cp::Action({_fact_checkedIn}, {_fact_hasCheckInPasword}));
  cp::Domain domain(actions);

  cp::Problem problem;
  _setGoalsForAPriority(problem, {_fact_beHappy});
  assert_eq<std::string>("", _lookForAnActionToDoConst(problem, domain));
}


void _triggerActionThatRemoveAFact()
{
  std::unique_ptr<std::chrono::steady_clock::time_point> now = {};
  std::map<std::string, cp::Action> actions;
  actions.emplace(_action_joke, cp::Action({_fact_beSad}, cp::WorldModification({}, {_fact_beSad})));
  actions.emplace(_action_goodBoy, cp::Action(cp::SetOfFacts({}, {_fact_beSad}), {_fact_beHappy}));

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
  cp::Action joke({cp::Fact::fromStr("isEngaged(human)")}, {cp::Fact::fromStr("isHappy(human)")});
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
  cp::Action joke({cp::Fact::fromStr("isEngaged(human)")}, {cp::Fact::fromStr("isHappy(human)")});
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

  actions.emplace(_action_welcome, cp::Action({cp::Fact::fromStr("place=entrance")}, {cp::Fact::fromStr("welcomePeople")}));

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
  actions.emplace(_action_askQuestion1, cp::Action({_fact_engagedWithUser}, {_fact_userSatisfied}, removeLearntBehavior));
  actions.emplace(_action_checkIn, cp::Action({}, cp::SetOfFacts::fromStr("!" + _fact_robotLearntABehavior + ", " + _fact_advertised, ',')));
  cp::Domain domain(actions);

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
  actions.emplace(_action_checkIn, cp::Action({_fact_engagedWithUser}, {_fact_checkedIn}));
  cp::Domain domain(actions);

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
  cp::Domain domain(actions);

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
  cp::Domain domain(actions);

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
  cp::Domain domain(actions);

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

  problem.changeGoalPriority(_fact_checkedIn, 9, true);
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
  problem.changeGoalPriority(_fact_checkedIn, 9, false);
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
  problem.changeGoalPriority(_fact_checkedIn, 9, true);
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
  actions.emplace(_action_checkIn, cp::Action({}, {_fact_checkedIn}));
  cp::Domain domain(actions);

  std::set<cp::Fact> factsChangedFromSubscription;
  cp::Problem problem;
  problem.addFact(_fact_beginOfConversation, now);
  auto factsChangedConnection = problem.onFactsChanged.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsChangedFromSubscription = pFacts;
  });
  cp::FactsChangedTracker factsChangedTracker(problem);
  std::set<cp::Fact> factsAdded;
  auto onFactsAddedConnection = factsChangedTracker.onFactsAdded.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsAdded = pFacts;
  });
  std::set<cp::Fact> factsRemoved;
  auto onFactsRemovedConnection = factsChangedTracker.onFactsRemoved.connectUnsafe([&](const std::set<cp::Fact>& pFacts) {
    factsRemoved = pFacts;
  });

  problem.setGoals({{9, {_fact_userSatisfied}}, {10, {_fact_greeted, _fact_checkedIn}}}, now);
  assert_eq({}, factsChangedFromSubscription);

  auto plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_greet, plannerResult.actionId);
  assert_eq({_fact_beginOfConversation, _fact_greeted}, factsChangedFromSubscription);
  assert_eq({_fact_greeted}, factsAdded);
  assert_eq({}, factsRemoved);

  plannerResult =_lookForAnActionToDoThenNotify(problem, domain);
  assert_eq<std::string>(_action_checkIn, plannerResult.actionId);
  assert_eq({_fact_beginOfConversation, _fact_greeted, _fact_checkedIn}, factsChangedFromSubscription);
  assert_eq({_fact_checkedIn}, factsAdded);
  assert_eq({}, factsRemoved);
  problem.removeFact(_fact_greeted, now);
  assert_eq({_fact_beginOfConversation, _fact_checkedIn}, factsChangedFromSubscription);
  assert_eq({_fact_checkedIn}, factsAdded);
  assert_eq({_fact_greeted}, factsRemoved);

  onFactsRemovedConnection.disconnect();
  onFactsAddedConnection.disconnect();
  factsChangedConnection.disconnect();
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
  _noPreconditionGoalImmediatlyReached();
  _removeFirstGoalsThatAreAlreadySatisfied();
  _removeAnAction();
  _removeSomeGoals();
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
  _checkShouldBeDoneAsap();
  _checkNotInAPrecondition();
  _checkClearGoalsWhenItsAlreadySatisfied();
  _fromAndToStrOfSetOfFacts();
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

  std::cout << "chatbot planner is ok !!!!" << std::endl;
  return 0;
}
