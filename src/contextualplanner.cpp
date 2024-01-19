#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>
#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>
#include "types/factsalreadychecked.hpp"


namespace cp
{

namespace
{

enum class PossibleEffect
{
  SATISFIED,
  SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD,
  NOT_SATISFIED
};

PossibleEffect _merge(PossibleEffect pEff1,
                      PossibleEffect pEff2)
{
  if (pEff1 == PossibleEffect::SATISFIED ||
      pEff2 == PossibleEffect::SATISFIED)
    return PossibleEffect::SATISFIED;
  if (pEff1 == PossibleEffect::SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD ||
      pEff2 == PossibleEffect::SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD)
    return PossibleEffect::SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD;
  return PossibleEffect::NOT_SATISFIED;
}

struct PotentialNextAction
{
  PotentialNextAction()
    : actionId(""),
      actionPtr(nullptr),
      parameters(),
      satisfyObjective(false)
  {
  }
  PotentialNextAction(const ActionId& pActionId,
                      const Action& pAction);

  ActionId actionId;
  const Action* actionPtr;
  std::map<std::string, std::set<std::string>> parameters;
  bool satisfyObjective;

  bool isMoreImportantThan(const PotentialNextAction& pOther,
                           const Problem& pProblem,
                           const Historical* pGlobalHistorical) const;
};


PotentialNextAction::PotentialNextAction(const ActionId& pActionId,
                                         const Action& pAction)
  : actionId(pActionId),
    actionPtr(&pAction),
    parameters(),
    satisfyObjective(false)
{
  for (const auto& currParam : pAction.parameters)
    parameters[currParam];
}


void _getPreferInContextStatistics(std::size_t& nbOfPreconditionsSatisfied,
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

  if (pAction.preferInContext)
    pAction.preferInContext->forAll(onFact);
}


bool PotentialNextAction::isMoreImportantThan(const PotentialNextAction& pOther,
                                              const Problem& pProblem,
                                              const Historical* pGlobalHistorical) const
{
  if (actionPtr == nullptr)
    return false;
  auto& action = *actionPtr;
  if (pOther.actionPtr == nullptr)
    return true;
  auto& otherAction = *pOther.actionPtr;

  auto nbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
  auto otherNbOfTimesAlreadyDone = pProblem.historical.getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);

  if (action.highImportanceOfNotRepeatingIt)
  {
    if (otherAction.highImportanceOfNotRepeatingIt)
    {
      if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
        return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;
    }
    else if (nbOfTimesAlreadyDone > 0)
    {
      return false;
    }
  }
  else if (otherAction.highImportanceOfNotRepeatingIt && otherNbOfTimesAlreadyDone > 0)
  {
    return true;
  }

  // Compare according to prefer in context
  std::size_t nbOfPreferInContextSatisfied = 0;
  std::size_t nbOfPreferInContextNotSatisfied = 0;
  _getPreferInContextStatistics(nbOfPreferInContextSatisfied, nbOfPreferInContextNotSatisfied, action, pProblem.worldState.facts());
  std::size_t otherNbOfPreconditionsSatisfied = 0;
  std::size_t otherNbOfPreconditionsNotSatisfied = 0;
  _getPreferInContextStatistics(otherNbOfPreconditionsSatisfied, otherNbOfPreconditionsNotSatisfied, otherAction, pProblem.worldState.facts());
  if (nbOfPreferInContextSatisfied != otherNbOfPreconditionsSatisfied)
    return nbOfPreferInContextSatisfied > otherNbOfPreconditionsSatisfied;
  if (nbOfPreferInContextNotSatisfied != otherNbOfPreconditionsNotSatisfied)
    return nbOfPreferInContextNotSatisfied < otherNbOfPreconditionsNotSatisfied;

  if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
    return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;

  if (pGlobalHistorical != nullptr)
  {
    nbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(actionId);
    otherNbOfTimesAlreadyDone = pGlobalHistorical->getNbOfTimeAnActionHasAlreadyBeenDone(pOther.actionId);
    if (nbOfTimesAlreadyDone != otherNbOfTimesAlreadyDone)
      return nbOfTimesAlreadyDone < otherNbOfTimesAlreadyDone;
  }
  return actionId < pOther.actionId;
}


bool _lookForAPossibleEffect(bool& pSatisfyObjective,
                             std::map<std::string, std::set<std::string>>& pParameters,
                             const ProblemModification& pEffectToCheck,
                             const Goal& pGoal,
                             const Problem& pProblem,
                             const FactOptional& pFactOptionalToSatisfy,
                             const Domain& pDomain,
                             FactsAlreadyChecked& pFactsAlreadychecked);


PossibleEffect _lookForAPossibleDeduction(const std::vector<std::string>& pParameters,
                                          const std::unique_ptr<Condition>& pCondition,
                                          const ProblemModification& pEffect,
                                          const FactOptional& pFactOptional,
                                          std::map<std::string, std::set<std::string>>& pParentParameters,
                                          const Goal& pGoal,
                                          const Problem& pProblem,
                                          const FactOptional& pFactOptionalToSatisfy,
                                          const Domain& pDomain,
                                          FactsAlreadyChecked& pFactsAlreadychecked)
{
  if (!pCondition ||
      (pCondition->containsFactOpt(pFactOptional, pParentParameters, pParameters) &&
       pCondition->canBecomeTrue(pProblem.worldState)))
  {
    std::map<std::string, std::set<std::string>> parametersToValues;
    for (const auto& currParam : pParameters)
      parametersToValues[currParam];
    bool satisfyObjective = false;
    if (_lookForAPossibleEffect(satisfyObjective, parametersToValues, pEffect, pGoal, pProblem, pFactOptionalToSatisfy,
                                pDomain, pFactsAlreadychecked))
    {
      bool actionIsAPossibleFollowUp = true;
      // fill parent parameters
      for (auto& currParentParam : pParentParameters)
      {
        if (currParentParam.second.empty())
        {
          pCondition->untilFalse(
                [&](const FactOptional& pConditionFactOptional)
          {
            auto parentParamValue = pFactOptional.fact.tryToExtractArgumentFromExample(currParentParam.first, pConditionFactOptional.fact);
            if (parentParamValue.empty())
              return true;
            // Maybe the extracted parameter is also a parameter so we replace by it's value
            auto itParam = parametersToValues.find(parentParamValue);
            if (itParam != parametersToValues.end())
              currParentParam.second = itParam->second;
            else
              currParentParam.second.insert(parentParamValue);
            return currParentParam.second.empty();
          }, pProblem.worldState, parametersToValues);

          if (!currParentParam.second.empty())
            break;
          if (currParentParam.second.empty())
          {
            actionIsAPossibleFollowUp = false;
            break;
          }
        }
      }
      // Check that the new fact pattern is not already satisfied
      if (actionIsAPossibleFollowUp)
      {
        if (!pProblem.worldState.isOptionalFactSatisfiedInASpecificContext(pFactOptional, {}, {}, &pParentParameters, nullptr))
          return PossibleEffect::SATISFIED;
        return PossibleEffect::SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD;
      }
    }
  }
  return PossibleEffect::NOT_SATISFIED;
}

PossibleEffect _lookForAPossibleExistingOrNotFactFromActions(
    const FactOptional& pFactOptional,
    std::map<std::string, std::set<std::string>>& pParentParameters,
    const std::map<std::string, std::set<ActionId>>& pPreconditionToActions,
    const Goal& pGoal,
    const Problem& pProblem,
    const FactOptional& pFactOptionalToSatisfy,
    const Domain& pDomain,
    FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto res = PossibleEffect::NOT_SATISFIED;
  auto it = pPreconditionToActions.find(pFactOptional.fact.name);
  if (it != pPreconditionToActions.end())
  {
    auto& actions = pDomain.actions();
    for (const auto& currActionId : it->second)
    {
      auto itAction = actions.find(currActionId);
      if (itAction != actions.end())
      {
        auto& action = itAction->second;
        res = _merge(_lookForAPossibleDeduction(action.parameters, action.precondition, action.effect,
                                                pFactOptional, pParentParameters, pGoal, pProblem, pFactOptionalToSatisfy,
                                                pDomain, pFactsAlreadychecked), res);
        if (res == PossibleEffect::SATISFIED)
          return res;
      }
    }
  }
  return res;
}


PossibleEffect _lookForAPossibleExistingOrNotFactFromInferences(
    const FactOptional& pFactOptional,
    std::map<std::string, std::set<std::string>>& pParentParameters,
    const std::map<std::string, std::set<InferenceId>>& pConditionToInferences,
    const std::map<InferenceId, Inference>& pInferences,
    const Goal& pGoal,
    const Problem& pProblem,
    const FactOptional& pFactOptionalToSatisfy,
    const Domain& pDomain,
    FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto res = PossibleEffect::NOT_SATISFIED;
  auto it = pConditionToInferences.find(pFactOptional.fact.name);
  if (it != pConditionToInferences.end())
  {
    for (const auto& currInferenceId : it->second)
    {
      auto itInference = pInferences.find(currInferenceId);
      if (itInference != pInferences.end())
      {
        auto& inference = itInference->second;
        if (inference.factsToModify)
        {
          res = _merge(_lookForAPossibleDeduction(inference.parameters, inference.condition, inference.factsToModify->clone(nullptr),
                                                  pFactOptional, pParentParameters, pGoal, pProblem, pFactOptionalToSatisfy,
                                                  pDomain, pFactsAlreadychecked), res);
          if (res == PossibleEffect::SATISFIED)
            return res;
        }
      }
    }
  }
  return res;
}


bool _lookForAPossibleEffect(bool& pSatisfyObjective,
                             std::map<std::string, std::set<std::string>>& pParameters,
                             const ProblemModification& pEffectToCheck,
                             const Goal& pGoal,
                             const Problem& pProblem,
                             const FactOptional& pFactOptionalToSatisfy,
                             const Domain& pDomain,
                             FactsAlreadyChecked& pFactsAlreadychecked)
{
  auto doesSatisfyObjective = [&](const FactOptional& pFactOptional)
  {
    if (pFactOptionalToSatisfy.isFactNegated != pFactOptional.isFactNegated)
      return false;
    std::map<std::string, std::set<std::string>> newParameters;
    bool res = pFactOptionalToSatisfy.fact.isInOtherFact(pFactOptional.fact, false, &newParameters, &pParameters);
    applyNewParams(pParameters, newParameters);
    return res;
  };

  if (pEffectToCheck.worldStateModification &&
      pEffectToCheck.worldStateModification->forAllUntilTrue(doesSatisfyObjective, pProblem.worldState))
  {
    pSatisfyObjective = true;
    return true;
  }
  if (pEffectToCheck.potentialWorldStateModification &&
      pEffectToCheck.potentialWorldStateModification->forAllUntilTrue(doesSatisfyObjective, pProblem.worldState))
  {
    pSatisfyObjective = true;
    return true;
  }

  auto& setOfInferences = pDomain.getSetOfInferences();
  return pEffectToCheck.forAllUntilTrue([&](const cp::FactOptional& pFactOptional) {
    // Condition only for optimization
    if (pParameters.empty())
    {
      if (!pFactOptional.isFactNegated)
      {
        if (pProblem.worldState.facts().count(pFactOptional.fact) > 0)
          return false;
      }
      else
      {
        if (pProblem.worldState.facts().count(pFactOptional.fact) == 0)
           return false;
      }
    }

    if ((!pFactOptional.isFactNegated && pFactsAlreadychecked.factsToAdd.count(pFactOptional.fact) == 0) ||
        (pFactOptional.isFactNegated && pFactsAlreadychecked.factsToRemove.count(pFactOptional.fact) == 0))
    {
      FactsAlreadyChecked subFactsAlreadychecked = pFactsAlreadychecked;
      if (!pFactOptional.isFactNegated)
        subFactsAlreadychecked.factsToAdd.insert(pFactOptional.fact);
      else
        subFactsAlreadychecked.factsToRemove.insert(pFactOptional.fact);

      auto& preconditionToActions = !pFactOptional.isFactNegated ? pDomain.preconditionToActions() : pDomain.notPreconditionToActions();
      PossibleEffect possibleEffect = _lookForAPossibleExistingOrNotFactFromActions(pFactOptional, pParameters, preconditionToActions, pGoal,
                                                                                    pProblem, pFactOptionalToSatisfy,
                                                                                    pDomain, subFactsAlreadychecked);
      if (possibleEffect != PossibleEffect::SATISFIED)
      {
        for (auto& currSetOfInferences : setOfInferences)
        {
          auto& inferences = currSetOfInferences.second.inferences();
          auto& conditionToReachableInferences = !pFactOptional.isFactNegated ?
                currSetOfInferences.second.reachableInferenceLinks().conditionToInferences :
                currSetOfInferences.second.reachableInferenceLinks().notConditionToInferences;
          possibleEffect = _merge(_lookForAPossibleExistingOrNotFactFromInferences(pFactOptional, pParameters, conditionToReachableInferences, inferences,
                                                                                   pGoal, pProblem, pFactOptionalToSatisfy,
                                                                                   pDomain, subFactsAlreadychecked), possibleEffect);
          if (possibleEffect == PossibleEffect::SATISFIED)
            break;
          auto& conditionToUnreachableInferences = !pFactOptional.isFactNegated ?
                currSetOfInferences.second.unreachableInferenceLinks().conditionToInferences :
                currSetOfInferences.second.unreachableInferenceLinks().notConditionToInferences;
          possibleEffect = _merge(_lookForAPossibleExistingOrNotFactFromInferences(pFactOptional, pParameters, conditionToUnreachableInferences, inferences,
                                                                                   pGoal, pProblem, pFactOptionalToSatisfy,
                                                                                   pDomain, subFactsAlreadychecked), possibleEffect);
          if (possibleEffect == PossibleEffect::SATISFIED)
            break;
        }
      }

      if (possibleEffect != PossibleEffect::SATISFIED_BUT_DOES_NOT_MODIFY_THE_WORLD)
        pFactsAlreadychecked.swap(subFactsAlreadychecked);
      return possibleEffect == PossibleEffect::SATISFIED;
    }
    return false;
  }, pProblem.worldState);
}


void _nextStepOfTheProblemForAGoalAndSetOfActions(PotentialNextAction& pCurrentResult,
                                                  const std::set<ActionId>& pActions,
                                                  const Goal& pGoal,
                                                  const Problem& pProblem,
                                                  const FactOptional& pFactOptionalToSatisfy,
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
      if (_lookForAPossibleEffect(newPotRes.satisfyObjective, newPotRes.parameters, action.effect, pGoal,
                                  pProblem, pFactOptionalToSatisfy, pDomain, factsAlreadyChecked) &&
          (!action.precondition || action.precondition->isTrue(pProblem.worldState, {}, {}, &newPotRes.parameters)))
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
    std::map<std::string, std::set<std::string>>& pParameters,
    const Goal& pGoal,
    const Problem& pProblem,
    const FactOptional& pFactOptionalToSatisfy,
    const Domain& pDomain,
    const Historical* pGlobalHistorical)
{
  PotentialNextAction res;
  for (const auto& currFact : pProblem.worldState.factNamesToFacts())
  {
    auto itPrecToActions = pDomain.preconditionToActions().find(currFact.first);
    if (itPrecToActions != pDomain.preconditionToActions().end())
      _nextStepOfTheProblemForAGoalAndSetOfActions(res, itPrecToActions->second, pGoal,
                                                   pProblem, pFactOptionalToSatisfy,
                                                   pDomain, pGlobalHistorical);
  }
  auto& actionsWithoutFactToAddInPrecondition = pDomain.actionsWithoutFactToAddInPrecondition();
  _nextStepOfTheProblemForAGoalAndSetOfActions(res, actionsWithoutFactToAddInPrecondition, pGoal,
                                               pProblem, pFactOptionalToSatisfy,
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
  pProblem.worldState.refreshCacheIfNeeded(pDomain);

  std::unique_ptr<OneStepOfPlannerResult> res;
  auto tryToFindAnActionTowardGoal = [&](Goal& pGoal, int pPriority){
    const FactOptional* factOptionalToSatisfyPtr = nullptr;
    pGoal.objective().untilFalse(
          [&](const FactOptional& pFactOptional)
    {
      if (!pProblem.worldState.isOptionalFactSatisfied(pFactOptional))
      {
        factOptionalToSatisfyPtr = &pFactOptional;
        return false;
      }
      return true;
    }, pProblem.worldState, {});

    if (factOptionalToSatisfyPtr != nullptr)
    {
      std::map<std::string, std::set<std::string>> parameters;
      auto actionId =
          _nextStepOfTheProblemForAGoal(parameters, pGoal,
                                        pProblem, *factOptionalToSatisfyPtr, pDomain, pGlobalHistorical);
      if (!actionId.empty())
      {
        res = std::make_unique<OneStepOfPlannerResult>(actionId, parameters, pGoal.clone(), pPriority);
        pGoal.notifyActivity();
        return true;
      }
      return false;
    }
    return false;
  };

  pProblem.goalStack.iterateOnGoalsAndRemoveNonPersistent(tryToFindAnActionTowardGoal, pProblem.worldState, pNow);
  return res;
}


void _notifyActionDone(Problem& pProblem,
                       const std::map<SetOfInferencesId, SetOfInferences>& pSetOfInferences,
                       const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                       const std::unique_ptr<WorldStateModification>& pEffect,
                       const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                       const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                       const std::vector<Goal>* pGoalsToAddInCurrentPriority)
{
  pProblem.historical.notifyActionDone(pOnStepOfPlannerResult.actionInstance.actionId);

  pProblem.worldState.notifyActionDone(pOnStepOfPlannerResult, pEffect, pProblem.goalStack, pSetOfInferences, pNow);

  pProblem.goalStack.notifyActionDone(pOnStepOfPlannerResult, pEffect, pNow, pGoalsToAdd,
                              pGoalsToAddInCurrentPriority, pProblem.worldState);
}


void notifyActionDone(Problem& pProblem,
                      const Domain& pDomain,
                      const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow)
{
  auto itAction = pDomain.actions().find(pOnStepOfPlannerResult.actionInstance.actionId);
  if (itAction != pDomain.actions().end())
  {
    auto& setOfInferences = pDomain.getSetOfInferences();
    _notifyActionDone(pProblem, setOfInferences, pOnStepOfPlannerResult, itAction->second.effect.worldStateModification, pNow,
                      &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);
  }
}




std::list<ActionInstance> lookForResolutionPlan(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical)
{
  std::set<std::string> actionAlreadyInPlan;
  std::list<ActionInstance> res;
  while (!pProblem.goalStack.goals().empty())
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
      auto& setOfInferences = pDomain.getSetOfInferences();
      _notifyActionDone(pProblem, setOfInferences, *onStepOfPlannerResult, itAction->second.effect.worldStateModification, pNow,
                        &itAction->second.effect.goalsToAdd, &itAction->second.effect.goalsToAddInCurrentPriority);

      if (itAction->second.effect.potentialFactsModifications)
      {
        auto potentialEffect = itAction->second.effect.potentialFactsModifications->cloneParamSet(onStepOfPlannerResult->actionInstance.parameters);
        pProblem.worldState.modifyFacts(potentialEffect, pProblem.goalStack, setOfInferences, pNow);
      }
    }
  }
  return res;
}


std::string planToStr(const std::list<cp::ActionInstance>& pPlan,
                      const std::string& pSep)
{
  auto size = pPlan.size();
  if (size == 1)
    return pPlan.front().toStr();
  std::string res;
  bool firstIteration = true;
  for (const auto& currAction : pPlan)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += pSep;
    res += currAction.toStr();
  }
  return res;
}


} // !cp
