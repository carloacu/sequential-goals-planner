#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>


namespace cp
{

namespace
{


struct FactsAlreadychecked
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


bool _doesFactInFacts(
    std::map<std::string, std::string>& pParameters,
    const Fact& pFact,
    const std::set<Fact>& pFacts,
    bool pParametersAreForTheFact)
{
  for (const auto& currFact : pFacts)
  {
    if (currFact.name != pFact.name ||
        currFact.parameters.size() != pFact.parameters.size())
      continue;

    auto doesItMatch = [&](const std::string& pFactValue, const std::string& pValueToLookFor) {
      if (pFactValue == pValueToLookFor)
        return true;
      if (pParameters.empty())
        return false;

      auto itParam = pParameters.find(pFactValue);
      if (itParam != pParameters.end())
      {
        if (!itParam->second.empty())
        {
          if (itParam->second == pValueToLookFor)
            return true;
        }
        else
        {
          pParameters[pFactValue] = pValueToLookFor;
          return true;
        }
      }
      return false;
    };

    {
      bool doesParametersMatches = true;
      auto itFactParameters = currFact.parameters.begin();
      auto itLookForParameters = pFact.parameters.begin();
      while (itFactParameters != currFact.parameters.end())
      {
        if (*itFactParameters != *itLookForParameters)
        {
          if (!itFactParameters->parameters.empty() ||
              !itFactParameters->value.empty() ||
              !itLookForParameters->parameters.empty() ||
              !itLookForParameters->value.empty() ||
              (!pParametersAreForTheFact && !doesItMatch(itFactParameters->name, itLookForParameters->name)) ||
              (pParametersAreForTheFact && !doesItMatch(itLookForParameters->name, itFactParameters->name)))
            doesParametersMatches = false;
        }
        ++itFactParameters;
        ++itLookForParameters;
      }
      if (!doesParametersMatches)
        continue;
    }

    if (pParametersAreForTheFact)
    {
      if (doesItMatch(pFact.value, currFact.value))
        return true;
    }
    else
    {
      if (doesItMatch(currFact.value, pFact.value))
        return true;
    }
  }
  return false;
}


bool _areFactsTrue(std::map<std::string, std::string>& pParameters,
                   const SetOfFacts& pSetOfFacts,
                   const Problem& pProblem)
{
  auto& facts = pProblem.facts();
  for (const auto& currFact : pSetOfFacts.facts)
    if (!_doesFactInFacts(pParameters, currFact, facts, true))
      return false;
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (_doesFactInFacts(pParameters, currFact, facts, true))
      return false;
  return areExpsValid(pSetOfFacts.exps, pProblem.variablesToValue());
}


bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const WorldModification& pEffectToCheck,
                             const Fact& pEffectToLookFor,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadychecked& pFactsAlreadychecked);


bool _lookForAPossibleExistingOrNotFact(
    const Fact& pFact,
    std::map<std::string, std::string>& pParentParameters,
    const std::map<std::string, std::set<ActionId>>& pPreconditionToActions,
    const Fact& pEffectToLookFor,
    const Problem& pProblem,
    const Domain& pDomain,
    FactsAlreadychecked& pFactsAlreadychecked)
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
            _lookForAPossibleEffect(parameters, action.effect, pEffectToLookFor, pProblem, pDomain, pFactsAlreadychecked))
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
                             const Fact& pEffectToLookFor,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadychecked& pFactsAlreadychecked)
{
  if (_doesFactInFacts(pParameters, pEffectToLookFor, pEffectToCheck.factsModifications.facts, false))
    return true;
  if (_doesFactInFacts(pParameters, pEffectToLookFor, pEffectToCheck.potentialFactsModifications.facts, false))
    return true;

  auto& preconditionToActions = pDomain.preconditionToActions();
  bool subRes = pEffectToCheck.forAllFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) == 0)
      if (_lookForAPossibleExistingOrNotFact(pFact, pParameters, preconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
    return false;
  });
  if (subRes)
    return true;

  auto& notPreconditionToActions = pDomain.notPreconditionToActions();
  return pEffectToCheck.forAllNotFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) > 0)
      if (_lookForAPossibleExistingOrNotFact(pFact, pParameters, notPreconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
    return false;
  });
}


bool _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Fact& pGoal,
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
      FactsAlreadychecked factsAlreadychecked;
      auto newPotRes = PotentialNextAction(currAction, action);
      if (_areFactsTrue(newPotRes.parameters, action.preconditions, pProblem) &&
          _lookForAPossibleEffect(newPotRes.parameters, action.effect, pGoal, pProblem, pDomain, factsAlreadychecked))
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
    const Fact& pGoal,
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
    auto& facts = pProblem.facts();

    auto& goalFact = pGoal.fact();
    if (facts.count(goalFact) == 0)
    {
      res = _nextStepOfTheProblemForAGoal(pParameters, goalFact, pProblem,
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


bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem)
{
  std::map<std::string, std::string> parameters;
  return _areFactsTrue(parameters, pSetOfFacts, pProblem);
}



} // !cp
