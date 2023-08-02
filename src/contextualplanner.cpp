#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>
#include <contextualplanner/types/setofinferences.hpp>

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
  auto onFact = [&](const FactOptional& pFactOptional)
  {
    if (pFactOptional.isFactNegated)
    {
      if (pFacts.count(pFactOptional.fact) == 0)
        ++nbOfPreconditionsSatisfied;
      else
        ++nbOfPreconditionsNotSatisfied;
    }
    else
    {
      if (pFacts.count(pFactOptional.fact) > 0)
        ++nbOfPreconditionsSatisfied;
      else
        ++nbOfPreconditionsNotSatisfied;
    }
  };

  if (pAction.precondition)
    pAction.precondition->forAll(onFact, [](const Expression&) {});
  if (pAction.preferInContext)
    pAction.preferInContext->forAll(onFact, [](const Expression&) {});
}


bool PotentialNextAction::isMoreImportantThan(const PotentialNextAction& pOther,
                                              const Problem& pProblem,
                                              const Historical* pGlobalHistorical) const
{
  if (actionPtr == nullptr)
    return false;
  if (pOther.actionPtr == nullptr)
    return true;

  auto nbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
  auto otherNbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
  if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
    return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;

  auto& action = *actionPtr;
  std::size_t nbOfPreconditionsSatisfied = 0;
  std::size_t nbOfPreconditionsNotSatisfied = 0;
  _getPrecoditionStatistics(nbOfPreconditionsSatisfied, nbOfPreconditionsNotSatisfied, action, pProblem.facts());
  auto& otherAction = *pOther.actionPtr;
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


bool _lookForAPossibleDeduction(const std::vector<std::string>& pParameters,
                                const std::unique_ptr<FactCondition>& pCondition,
                                const WorldModification& pEffect,
                                const Fact& pFact,
                                std::map<std::string, std::string>& pParentParameters,
                                const Goal& pGoal,
                                const Problem& pProblem,
                                const Domain& pDomain,
                                FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pCondition || pCondition->canBecomeTrue(pProblem))
  {
    std::map<std::string, std::string> parametersToValue;
    for (const auto& currParam : pParameters)
      parametersToValue[currParam];
    if (_lookForAPossibleEffect(parametersToValue, pEffect, pGoal, pProblem, pDomain, pFactsAlreadychecked))
    {
      bool actionIsAPossibleFollowUp = true;
      // fill parent parameters
      for (auto& currParentParam : pParentParameters)
      {
        if (currParentParam.second.empty())
        {
          pCondition->untilFalse(
                [&](const FactOptional& pFactOptional)
          {
            if (!pFactOptional.isFactNegated)
            {
              currParentParam.second = pFact.tryToExtractParameterValueFromExemple(currParentParam.first, pFactOptional.fact);
              if (!currParentParam.second.empty())
                return false;
            }
            return true;
          },
          [](const Expression&) { return true; }, pProblem);

          if (!currParentParam.second.empty())
            break;
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
  return false;
}

bool _lookForAPossibleExistingOrNotFactFromActions(
    const Fact& pFact,
    std::map<std::string, std::string>& pParentParameters,
    const std::map<std::string, std::set<ActionId>>& pPreconditionToActions,
    const Goal& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto it = pPreconditionToActions.find(pFact.name);
  if (it != pPreconditionToActions.end())
  {
    auto& actions = pDomain.actions();
    for (const auto& currActionId : it->second)
    {
      auto itAction = actions.find(currActionId);
      if (itAction != actions.end())
      {
        auto& action = itAction->second;
        if (_lookForAPossibleDeduction(action.parameters, action.precondition, action.effect,
                                       pFact, pParentParameters, pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
      }
    }
  }
  return false;
}


bool _lookForAPossibleExistingOrNotFactFromInferences(
    const Fact& pFact,
    std::map<std::string, std::string>& pParentParameters,
    const std::map<std::string, std::set<InferenceId>>& pConditionToInferences,
    const std::map<InferenceId, Inference>& pInferences,
    const Goal& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto it = pConditionToInferences.find(pFact.name);
  if (it != pConditionToInferences.end())
  {
    for (const auto& currInferenceId : it->second)
    {
      auto itInference = pInferences.find(currInferenceId);
      if (itInference != pInferences.end())
      {
        auto& inference = itInference->second;
        std::vector<std::string> parameters;
        if (inference.factsToModify &&
            _lookForAPossibleDeduction(parameters, inference.condition, inference.factsToModify->clone(nullptr),
                                       pFact, pParentParameters, pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
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
  if (!optionalFactGoal.isFactNegated)
  {
    if (pEffectToCheck.factsModifications &&
        pEffectToCheck.factsModifications->forAllFactsUntilTrue(
              [&](const Fact& pFact)
        {
          return optionalFactGoal.fact.isInFact(pFact, false, &pParameters);
        }, pProblem))
    {
      return true;
    }
    if (pEffectToCheck.potentialFactsModifications &&
        pEffectToCheck.potentialFactsModifications->forAllFactsUntilTrue(
              [&](const Fact& pFact)
        {
          return optionalFactGoal.fact.isInFact(pFact, false, &pParameters);
        }, pProblem))
    {
      return true;
    }
  }
  else
  {
    if (pEffectToCheck.factsModifications &&
        pEffectToCheck.factsModifications->forAllNotFactsUntilTrue(
              [&](const Fact& pFact)
        {
          return optionalFactGoal.fact.isInFact(pFact, false, &pParameters);
        }, pProblem))
    {
      return true;
    }
    if (pEffectToCheck.potentialFactsModifications &&
        pEffectToCheck.potentialFactsModifications->forAllNotFactsUntilTrue(
              [&](const Fact& pFact)
        {
          return optionalFactGoal.fact.isInFact(pFact, false, &pParameters);
        }, pProblem))
    {
      return true;
    }
  }

  auto& setOfInferences = pProblem.getSetOfInferences();
  auto& preconditionToActions = pDomain.preconditionToActions();
  bool subRes = pEffectToCheck.forAllFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) == 0 &&
        pFactsAlreadychecked.factsToAdd.insert(pFact).second)
    {
      if (_lookForAPossibleExistingOrNotFactFromActions(pFact, pParameters, preconditionToActions, pGoal,
                                                        pProblem, pDomain, pFactsAlreadychecked))
        return true;

      for (auto& currSetOfInferences : setOfInferences)
      {
        auto& inferences = currSetOfInferences.second->inferences();
        auto& conditionToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().conditionToInferences;
        if (_lookForAPossibleExistingOrNotFactFromInferences(pFact, pParameters, conditionToReachableInferences, inferences,
                                                             pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
        auto& conditionToUnreachableInferences = currSetOfInferences.second->unreachableInferenceLinks().conditionToInferences;
        if (_lookForAPossibleExistingOrNotFactFromInferences(pFact, pParameters, conditionToUnreachableInferences, inferences,
                                                             pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
      }
    }
    return false;
  }, pProblem);
  if (subRes)
    return true;

  auto& notPreconditionToActions = pDomain.notPreconditionToActions();
  return pEffectToCheck.forAllNotFactsUntilTrue([&](const cp::Fact& pFact) {
    if (pProblem.facts().count(pFact) > 0 &&
        pFactsAlreadychecked.factsToRemove.insert(pFact).second)
    {
      if (_lookForAPossibleExistingOrNotFactFromActions(pFact, pParameters, notPreconditionToActions, pGoal,
                                                        pProblem, pDomain, pFactsAlreadychecked))
        return true;

      for (auto& currSetOfInferences : setOfInferences)
      {
        auto& inferences = currSetOfInferences.second->inferences();
        auto& notConditionToReachableInferences = currSetOfInferences.second->reachableInferenceLinks().notConditionToInferences;
        if (_lookForAPossibleExistingOrNotFactFromInferences(pFact, pParameters, notConditionToReachableInferences, inferences,
                                                             pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
        auto& notConditionToUnreachableInferences = currSetOfInferences.second->unreachableInferenceLinks().notConditionToInferences;
        if (_lookForAPossibleExistingOrNotFactFromInferences(pFact, pParameters, notConditionToUnreachableInferences, inferences,
                                                             pGoal, pProblem, pDomain, pFactsAlreadychecked))
          return true;
      }
    }
    return false;
  }, pProblem);
}


void _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Goal& pGoal,
                                                  const Problem& pProblem,
                                                  const Domain& pDomain,
                                                  const Historical* pGlobalHistorical)
{
  PotentialNextAction newPotNextAction;
  auto& domainActions = pDomain.actions();
  for (const auto& currAction : pActions)
  {
    auto itAction = domainActions.find(currAction);
    if (itAction != domainActions.end())
    {
      auto& action = itAction->second;
      FactsAlreadyChecked factsAlreadyChecked;
      auto newPotRes = PotentialNextAction(currAction, action);
      if (pProblem.isConditionTrue(action.precondition, {}, &newPotRes.parameters) &&
          _lookForAPossibleEffect(newPotRes.parameters, action.effect, pGoal, pProblem, pDomain, factsAlreadyChecked))
      {
        if (newPotRes.isMoreImportantThan(newPotNextAction, pProblem, pGlobalHistorical))
        {
          assert(newPotRes.actionPtr != nullptr);
          newPotNextAction = newPotRes;
        }
      }
    }
  }

  if (newPotNextAction.isMoreImportantThan(pCurrentResult, pProblem, pGlobalHistorical))
  {
    assert(newPotNextAction.actionPtr != nullptr);
    pCurrentResult = newPotNextAction;
  }
}


ActionId _nextStepOfTheProblemForAGoal(
    std::map<std::string, std::string>& pParameters,
    const Goal& pGoal,
    const Problem& pProblem,
    const Domain& pDomain,
    const Historical* pGlobalHistorical)
{
  PotentialNextAction res;
  for (const auto& currFact : pProblem.factNamesToFacts())
  {
    auto itPrecToActions = pDomain.preconditionToActions().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActions().end())
      _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                   pDomain, pGlobalHistorical);
  }
  for (const auto& currFact : pProblem.variablesToValue())
  {
    auto itPrecToActions = pDomain.preconditionToActionsExps().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActionsExps().end())
      _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal, pProblem,
                                                   pDomain, pGlobalHistorical);
  }
  auto& actionsWithoutFactToAddInPrecondition = pDomain.actionsWithoutFactToAddInPrecondition();
  _nextStepOfTheProblemForAGoalAndSetOfActions(res, actionsWithoutFactToAddInPrecondition, pGoal, pProblem,
                                               pDomain, pGlobalHistorical);
  pParameters = std::move(res.parameters);
  return res.actionId;
}


}


std::unique_ptr<OneStepOfPlannerResult> lookForAnActionToDo(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    const Historical* pGlobalHistorical)
{
  pProblem.fillAccessibleFacts(pDomain);

  std::unique_ptr<OneStepOfPlannerResult> res;
  auto tryToFindAnActionTowardGoal = [&](Goal& pGoal, int pPriority){
    if (!pProblem.isOptionalFactSatisfied(pGoal.factOptional()))
    {
      std::map<std::string, std::string> parameters;
      auto actionId =
          _nextStepOfTheProblemForAGoal(parameters,
                                        pGoal, pProblem, pDomain, pGlobalHistorical);
      if (!actionId.empty())
      {
        res = std::make_unique<OneStepOfPlannerResult>(actionId, parameters, pGoal, pPriority);
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
                      const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  auto itAction = pDomain.actions().find(pOnStepOfPlannerResult.actionInstance.actionId);
  if (itAction != pDomain.actions().end())
    pProblem.notifyActionDone(pOnStepOfPlannerResult, itAction->second.effect.factsModifications, pNow,
                              &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);
}


std::list<ActionInstance> lookForResolutionPlan(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical)
{
  std::set<std::string> actionAlreadyInPlan;
  std::list<ActionInstance> res;
  while (!pProblem.goals().empty())
  {
    auto onStepOfPlannerResult = lookForAnActionToDo(pProblem, pDomain, pNow, pGlobalHistorical);
    if (!onStepOfPlannerResult)
      break;
    res.emplace_back(onStepOfPlannerResult->actionInstance);
    const auto& actionToDo = onStepOfPlannerResult->actionInstance.actionId;
    if (actionAlreadyInPlan.count(actionToDo) > 0)
      break;
    actionAlreadyInPlan.insert(actionToDo);

    auto itAction = pDomain.actions().find(actionToDo);
    if (itAction != pDomain.actions().end())
    {
      if (pGlobalHistorical != nullptr)
        pGlobalHistorical->notifyActionDone(actionToDo);
      pProblem.notifyActionDone(*onStepOfPlannerResult, itAction->second.effect.factsModifications, pNow,
                                &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);
    }
  }
  return res;
}


} // !cp
