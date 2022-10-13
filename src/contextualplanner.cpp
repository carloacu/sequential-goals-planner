#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>


namespace cp
{

namespace
{


struct FactsAlreadyChecked
{
  std::set<Fact> factsToAdd;
  std::set<Fact> factsToRemove;
};


struct PotentialNextAction
{
  PotentialNextAction()
    : actionId(""),
      actionPtr(nullptr),
      parameters()
  {
  }
  PotentialNextAction(const ActionId& pActionId,
                      const Action& pAction);

  ActionId actionId;
  const Action* actionPtr;
  std::map<std::string, std::string> parameters;

  bool isMoreImportantThan(const PotentialNextAction& pOther,
                           const Problem& pProblem,
                           const Historical* pGlobalHistorical) const;
};


PotentialNextAction::PotentialNextAction(const ActionId& pActionId,
                                         const Action& pAction)
  : actionId(pActionId),
    actionPtr(&pAction),
    parameters()
{
  for (const auto& currParam : pAction.parameters)
    parameters[currParam];
}


void _getPrecoditionStatistics(std::size_t& nbOfPreconditionsSatisfied,
                               std::size_t& nbOfPreconditionsNotSatisfied,
                               const Action& pAction,
                               const std::set<Fact>& pFacts)
{
  nbOfPreconditionsSatisfied = pAction.preconditions.facts.size();
  for (const auto& currFact : pAction.preferInContext.facts)
  {
    if (pFacts.count(currFact) > 0)
      ++nbOfPreconditionsSatisfied;
    else
      ++nbOfPreconditionsNotSatisfied;
  }
  for (const auto& currFact : pAction.preferInContext.notFacts)
  {
    if (pFacts.count(currFact) == 0)
      ++nbOfPreconditionsSatisfied;
    else
      ++nbOfPreconditionsNotSatisfied;
  }
}


bool PotentialNextAction::isMoreImportantThan(const PotentialNextAction& pOther,
                                              const Problem& pProblem,
                                              const Historical* pGlobalHistorical) const
{
  if (actionPtr == nullptr)
    return false;
  if (pOther.actionPtr == nullptr)
    return true;
  auto& action = *actionPtr;
  if (action.shouldBeDoneAsapWithoutHistoryCheck)
    return true;
  auto& otherAction = *pOther.actionPtr;

  auto nbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
  auto otherNbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
  if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
    return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;

  std::size_t nbOfPreconditionsSatisfied = 0;
  std::size_t nbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(nbOfPreconditionsSatisfied, nbOfPreconditionsNotSatisfied, action, pProblem.facts());
  std::size_t otherNbOfPreconditionsSatisfied = 0;
  std::size_t otherNbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(otherNbOfPreconditionsSatisfied, otherNbOfPreconditionsNotSatisfied, otherAction, pProblem.facts());
  if (nbOfPreconditionsSatisfied != otherNbOfPreconditionsSatisfied)
    return nbOfPreconditionsSatisfied > otherNbOfPreconditionsSatisfied;
  if (nbOfPreconditionsNotSatisfied != otherNbOfPreconditionsNotSatisfied)
    return nbOfPreconditionsNotSatisfied < otherNbOfPreconditionsNotSatisfied;

  if (pGlobalHistorical != nullptr)
  {
    nbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
    otherNbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
    if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
      return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;
  }

  return actionId < pOther.actionId;
}


bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const WorldModification& pEffectToCheck,
                             const Goal &pGoal,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadyChecked& pFactsAlreadychecked);


bool _lookForAPossibleExistingOrNotFact(
    const Fact& pFact,
    std::map<std::string, std::string>& pParentParameters,
    const std::map<std::string, std::set<ActionId>>& pPreconditionToActions,
    const Goal& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    return false;
  auto it = pPreconditionToActions.find(pFact.name);
  if (it != pPreconditionToActions.end())
  {
    for (const auto& currActionId : it->second)
    {
      auto itAction = pDomain.actions().find(currActionId);
      if (itAction != pDomain.actions().end())
      {
        auto& action = itAction->second;
        std::map<std::string, std::string> parameters;
        for (const auto& currParam : action.parameters)
          parameters[currParam];
        if (pProblem.canFactsBecomeTrue(action.preconditions) &&
            _lookForAPossibleEffect(parameters, action.effect, pGoal, pProblem, pDomain, pFactsAlreadychecked))
        {
          bool actionIsAPossibleFollowUp = true;
          // fill parent parameters
          for (auto& currParentParam : pParentParameters)
          {
            if (currParentParam.second.empty())
            {
              //for (const auto& currFact : pFacts)
              {
                for (const auto& currActionPreconditionFact : action.preconditions.facts)
                {
                  currParentParam.second = /*currFact*/ pFact.tryToExtractParameterValueFromExemple(currParentParam.first, currActionPreconditionFact);
                  if (!currParentParam.second.empty())
                    break;
                }
                if (!currParentParam.second.empty())
                  break;
              }
              if (currParentParam.second.empty())
              {
                actionIsAPossibleFollowUp = false;
                break;
              }
            }
          }
          if (actionIsAPossibleFollowUp)
            return true;
        }
      }
    }
  }
  return false;
}



bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const WorldModification& pEffectToCheck,
                             const Goal& pGoal,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto& optionalFactGoal = pGoal.factOptional();
  bool goalFactIsInEffect = optionalFactGoal.fact.isInFacts(pEffectToCheck.factsModifications.facts, false, &pParameters) ||
      optionalFactGoal.fact.isInFacts(pEffectToCheck.potentialFactsModifications.facts, false, &pParameters);
  if (!optionalFactGoal.isFactNegated)
  {
    if (goalFactIsInEffect)
      return true;
  }
  else
  {
    if (!goalFactIsInEffect)
      return true;
  }

  auto& preconditionToActions = pDomain.preconditionToActions();
  bool subRes = pEffectToCheck.forAllFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) == 0)
      if (_lookForAPossibleExistingOrNotFact(pFact, pParameters, preconditionToActions, pGoal,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
    return false;
  });
  if (subRes)
    return true;

  auto& notPreconditionToActions = pDomain.notPreconditionToActions();
  return pEffectToCheck.forAllNotFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) > 0)
      if (_lookForAPossibleExistingOrNotFact(pFact, pParameters, notPreconditionToActions, pGoal,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
    return false;
  });
}


bool _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Goal& pGoal,
                                                  const Problem& pProblem,
                                                  const Domain& pDomain,
                                                  const Historical* pGlobalHistorical)
{
  PotentialNextAction newPotNextAction;
  for (const auto& currAction : pActions)
  {
    auto itAction = pDomain.actions().find(currAction);
    if (itAction != pDomain.actions().end())
    {
      auto& action = itAction->second;
      FactsAlreadyChecked factsAlreadyChecked;
      auto newPotRes = PotentialNextAction(currAction, action);
      if (pProblem.areFactsTrue(action.preconditions, &newPotRes.parameters) &&
          _lookForAPossibleEffect(newPotRes.parameters, action.effect, pGoal, pProblem, pDomain, factsAlreadyChecked))
      {
        if (newPotRes.isMoreImportantThan(newPotNextAction, pProblem, pGlobalHistorical))
        {
          assert(newPotRes.actionPtr != nullptr);
          newPotNextAction = newPotRes;
          if (newPotRes.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
            break;
        }
      }
    }
  }

  if (newPotNextAction.isMoreImportantThan(pCurrentResult, pProblem, pGlobalHistorical))
  {
    assert(newPotNextAction.actionPtr != nullptr);
    pCurrentResult = newPotNextAction;
    if (newPotNextAction.actionPtr->shouldBeDoneAsapWithoutHistoryCheck)
      return true;
  }
  return false;
}


ActionId _nextStepOfTheProblemForAGoal(
    std::map<std::string, std::string>& pParameters,
    const Goal& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    const Historical* pGlobalHistorical)
{
  PotentialNextAction res;
  for (const auto& currFact : pProblem.factNamesToNbOfOccurences())
  {
    auto itPrecToActions = pDomain.preconditionToActions().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActions().end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                     pDomain, pGlobalHistorical))
    {
      pParameters = std::move(res.parameters);
      return res.actionId;
    }
  }
  for (const auto& currFact : pProblem.variablesToValue())
  {
    auto itPrecToActions = pDomain.preconditionToActionsExps().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActionsExps().end() &&
        _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                     pDomain, pGlobalHistorical))
    {
      pParameters = std::move(res.parameters);
      return res.actionId;
    }
  }
  auto& actionsWithoutPrecondition = pDomain.actionsWithoutPrecondition();
  _nextStepOfTheProblemForAGoalAndSetOfActions(res, actionsWithoutPrecondition, pGoal, pProblem,
                                               pDomain, pGlobalHistorical);
  pParameters = std::move(res.parameters);
  return res.actionId;
}


}


ActionId lookForAnActionToDo(std::map<std::string, std::string>& pParameters,
                             Problem& pProblem,
                             const Domain& pDomain,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                             const Goal** pGoalOfTheAction,
                             int* pGoalPriority,
                             const Historical* pGlobalHistorical)
{
  pProblem.fillReachableFacts(pDomain);

  ActionId res;
  auto tryToFindAnActionTowardGoal = [&](Goal& pGoal, int pPriority){
    if (!pProblem.isOptionalFactSatisfied(pGoal.factOptional()))
    {
      res = _nextStepOfTheProblemForAGoal(pParameters, pGoal, pProblem,
                                          pDomain, pGlobalHistorical);
      if (!res.empty())
      {
        if (pGoalPriority != nullptr)
          *pGoalPriority = pPriority;
        if (pGoalOfTheAction != nullptr)
          *pGoalOfTheAction = &pGoal;
        pGoal.notifyActivity();
        return true;
      }
      return false;
    }
    return false;
  };

  pProblem.iterateOnGoalsAndRemoveNonPersistent(tryToFindAnActionTowardGoal, pNow);
  return res;
}



void notifyActionDone(Problem& pProblem,
                      const Domain& pDomain,
                      const std::string& pActionId,
                      const std::map<std::string, std::string>& pParameters,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  auto itAction = pDomain.actions().find(pActionId);
  if (itAction != pDomain.actions().end())
    pProblem.notifyActionDone(pActionId, pParameters, itAction->second.effect.factsModifications,
                              pNow, &itAction->second.effect.goalsToAdd);
}


} // !cp
