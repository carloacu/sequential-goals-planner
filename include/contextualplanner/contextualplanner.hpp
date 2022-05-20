#ifndef INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
#define INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP

#include <map>
#include <set>
#include <list>
#include <vector>
#include <assert.h>
#include "api.hpp"
#include <contextualplanner/alias.hpp>
#include <contextualplanner/problem.hpp>



namespace cp
{


struct CONTEXTUALPLANNER_API Action
{
  Action(const SetOfFacts& pPreconditions,
         const SetOfFacts& pEffects,
         const SetOfFacts& pPreferInContext = {},
         bool pShouldBeDoneAsapWithoutHistoryCheck = false)
    : parameters(),
      preconditions(pPreconditions),
      preferInContext(pPreferInContext),
      effects(pEffects),
      shouldBeDoneAsapWithoutHistoryCheck(pShouldBeDoneAsapWithoutHistoryCheck)
  {
  }

  std::vector<std::string> parameters;
  SetOfFacts preconditions;
  SetOfFacts preferInContext;
  SetOfFacts effects;
  // If this it's true it will have a very high priority for the planner.
  // It is approriate to use that for deduction actions.
  bool shouldBeDoneAsapWithoutHistoryCheck;
};

struct CONTEXTUALPLANNER_API Domain
{
  Domain(const std::map<ActionId, Action>& pActions);

  void addAction(ActionId pActionId,
                 const Action& pAction);
  void removeAction(ActionId pActionId);

  const std::map<ActionId, Action>& actions() const { return _actions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActions() const { return _preconditionToActions; }
  const std::map<std::string, std::set<ActionId>>& preconditionToActionsExps() const { return _preconditionToActionsExps; }
  const std::map<std::string, std::set<ActionId>>& notPreconditionToActions() const { return _notPreconditionToActions; }
  const std::set<ActionId>& actionsWithoutPrecondition() const { return _actionsWithoutPrecondition; }

private:
  std::map<ActionId, Action> _actions;
  std::map<std::string, std::set<ActionId>> _preconditionToActions;
  std::map<std::string, std::set<ActionId>> _preconditionToActionsExps;
  std::map<std::string, std::set<ActionId>> _notPreconditionToActions;
  std::set<ActionId> _actionsWithoutPrecondition;
};




CONTEXTUALPLANNER_API
void replaceVariables(std::string& pStr,
                      const Problem& pProblem);


CONTEXTUALPLANNER_API
void fillReachableFacts(Problem& pProblem,
                        const Domain& pDomain);


CONTEXTUALPLANNER_API
bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem);

CONTEXTUALPLANNER_API
ActionId lookForAnActionToDo(std::map<std::string, std::string>& pParameters,
                             Problem& pProblem,
                             const Domain& pDomain,
                             const Historical* pGlobalHistorical = nullptr);

CONTEXTUALPLANNER_API
std::string printActionIdWithParameters(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters);

CONTEXTUALPLANNER_API
std::list<ActionId> solve(Problem& pProblem,
                          const Domain& pDomain,
                          Historical* pGlobalHistorical = nullptr);
} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
