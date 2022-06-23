#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>
#include <contextualplanner/arithmeticevaluator.hpp>


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

std::string _expressionEltToValue(const ExpressionElement& pExpElt,
                                  const std::map<std::string, std::string>& pVariablesToValue)
{
  if (pExpElt.type == ExpressionElementType::FACT)
  {
    auto it = pVariablesToValue.find(pExpElt.value);
    if (it != pVariablesToValue.end())
      return it->second;
    return "";
  }
  return pExpElt.value;
}

bool _areExpsValid(const std::list<Expression>& pExps,
                   const std::map<std::string, std::string>& pVariablesToValue)
{
  for (const auto& currExp : pExps)
  {
    if (currExp.elts.size() >= 3)
    {
      auto it = currExp.elts.begin();
      auto val1 = _expressionEltToValue(*it, pVariablesToValue);
      ++it;
      if (it->type != ExpressionElementType::OPERATOR ||
          it->value != "=")
        return false;
      ++it;
      auto val2 = _expressionEltToValue(*it, pVariablesToValue);
      ++it;
      while (it != currExp.elts.end())
      {
        if (it->type != ExpressionElementType::OPERATOR)
          return false;
        auto op = it->value;
        ++it;
        if (it == currExp.elts.end())
          break;
        auto val3 = _expressionEltToValue(*it, pVariablesToValue);
        if (op == "+" || op == "-")
          val2 = evaluteToStr(val2 + op + val3);
        else
          return false;
        ++it;
      }
      if (it != currExp.elts.end() || val1 != val2)
        return false;
    }
  }
  return true;
}


bool _canFactsBecomeTrue(const SetOfFacts& pSetOfFacts,
                         const Problem& pProblem)
{
  auto& facts = pProblem.facts();
  auto& reachableFacts = pProblem.reachableFacts();
  auto& reachableFactsWithAnyValues = pProblem.reachableFactsWithAnyValues();
  for (const auto& currFact : pSetOfFacts.facts)
  {
    if (facts.count(currFact) == 0 &&
        reachableFacts.count(currFact) == 0)
    {
      bool reableFactFound = false;
      for (const auto& currReachableFact : reachableFactsWithAnyValues)
      {
        if (currFact.areEqualExceptAnyValues(currReachableFact))
        {
          reableFactFound = true;
          break;
        }
      }
      if (!reableFactFound)
        return false;
    }
  }
  auto& removableFacts = pProblem.removableFacts();
  for (const auto& currFact : pSetOfFacts.notFacts)
    if (facts.count(currFact) > 0 &&
        removableFacts.count(currFact) == 0)
      return false;
  return _areExpsValid(pSetOfFacts.exps, pProblem.variablesToValue());
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
  for (const auto& currPrecond : pSetOfFacts.facts)
    if (!_doesFactInFacts(pParameters, currPrecond, facts, true))
      return false;
  for (const auto& currPrecond : pSetOfFacts.notFacts)
    if (_doesFactInFacts(pParameters, currPrecond, facts, true))
      return false;
  return _areExpsValid(pSetOfFacts.exps, pProblem.variablesToValue());
}


void _getTheFactsToAddFromTheActionEffects(std::set<Fact>& pNewFacts,
                                           std::vector<Fact>& pNewFactsWithAnyValues,
                                           const Action& pAction,
                                           const std::set<Fact>& pFacts1,
                                           const std::set<Fact>& pFacts2)
{
  for (const auto& currFact : pAction.effects.facts)
  {
    if (pFacts1.count(currFact) == 0 &&
        pFacts2.count(currFact) == 0)
    {
      auto factToInsert = currFact;
      if (factToInsert.replaceParametersByAny(pAction.parameters))
        pNewFactsWithAnyValues.push_back(std::move(factToInsert));
      else
        pNewFacts.insert(std::move(factToInsert));
    }
  }
}

void _getTheFactsToRemoveFromTheActionEffects(std::set<Fact>& pFactsToRemove,
                                              const Action& pAction,
                                              const std::set<Fact>& pFacts1,
                                              const std::set<Fact>& pFacts2)
{
  for (const auto& currPrecond : pAction.effects.notFacts)
    if (pFacts1.count(currPrecond) > 0 &&
        pFacts2.count(currPrecond) == 0)
      pFactsToRemove.insert(currPrecond);
}

bool _lookForAPossibleEffect(std::map<std::string, std::string>& pParameters,
                             const SetOfFacts& pEffectsToCheck,
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
        if (_canFactsBecomeTrue(action.preconditions, pProblem) &&
            _lookForAPossibleEffect(parameters, action.effects, pEffectToLookFor, pProblem, pDomain, pFactsAlreadychecked))
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
                             const SetOfFacts& pEffectsToCheck,
                             const Fact& pEffectToLookFor,
                             const Problem& pProblem,
                             const Domain& pDomain,
                             FactsAlreadychecked& pFactsAlreadychecked)
{
  if (_doesFactInFacts(pParameters, pEffectToLookFor, pEffectsToCheck.facts, false))
    return true;

  auto& preconditionToActions = pDomain.preconditionToActions();
  for (auto& currFact : pEffectsToCheck.facts)
    if (pProblem.facts().count(currFact) == 0)
      if (_lookForAPossibleExistingOrNotFact(currFact, pParameters, preconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
        return true;
  auto& notPreconditionToActions = pDomain.notPreconditionToActions();
  for (auto& currFact : pEffectsToCheck.notFacts)
    if (pProblem.facts().count(currFact) > 0)
      if (_lookForAPossibleExistingOrNotFact(currFact, pParameters, notPreconditionToActions, pEffectToLookFor,
                                             pProblem, pDomain, pFactsAlreadychecked))
      return true;
  return false;
}

void _feedReachableFacts(Problem& pProblem,
                         const Fact& pFact,
                         const Domain& pDomain);

void _feedReachableFactsFromSetOfActions(Problem& pProblem,
                                         const std::set<ActionId>& pActions,
                                         const Domain& pDomain)
{
  for (const auto& currAction : pActions)
  {
    auto itAction = pDomain.actions().find(currAction);
    if (itAction != pDomain.actions().end())
    {
      auto& action = itAction->second;
      if (_canFactsBecomeTrue(action.preconditions, pProblem))
      {
        std::set<Fact> reachableFactsToAdd;
        std::vector<Fact> reachableFactsToAddWithAnyValues;
        _getTheFactsToAddFromTheActionEffects(reachableFactsToAdd, reachableFactsToAddWithAnyValues, action, pProblem.facts(), pProblem.reachableFacts());
        std::set<Fact> removableFactsToAdd;
        _getTheFactsToRemoveFromTheActionEffects(removableFactsToAdd, action, pProblem.facts(), pProblem.removableFacts());
        if (!reachableFactsToAdd.empty() || !reachableFactsToAddWithAnyValues.empty() || !removableFactsToAdd.empty())
        {
          pProblem.addReachableFacts(reachableFactsToAdd);
          pProblem.addReachableFactsWithAnyValues(reachableFactsToAddWithAnyValues);
          pProblem.addRemovableFacts(removableFactsToAdd);
          for (const auto& currNewFact : reachableFactsToAdd)
            _feedReachableFacts(pProblem, currNewFact, pDomain);
          for (const auto& currNewFact : removableFactsToAdd)
            _feedReachableFacts(pProblem, currNewFact, pDomain);
        }
      }
    }
  }
}


void _feedReachableFacts(Problem& pProblem,
                         const Fact& pFact,
                         const Domain& pDomain)
{
  auto itPrecToActions = pDomain.preconditionToActions().find(pFact.name);
  if (itPrecToActions != pDomain.preconditionToActions().end())
    _feedReachableFactsFromSetOfActions(pProblem, itPrecToActions->second, pDomain);
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
          _lookForAPossibleEffect(newPotRes.parameters, action.effects, pGoal, pProblem, pDomain, factsAlreadychecked))
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
  for (const auto& currFact : pProblem.factNamesToNbOfFactOccurences())
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


void replaceVariables(std::string& pStr,
                      const Problem& pProblem)
{
  // replace variable
  auto currentPos = pStr.find_first_of("${");
  while (currentPos != std::string::npos)
  {
    auto beginOfVarName = currentPos + 2;
    auto endVarPos = pStr.find("}", beginOfVarName);
    if (endVarPos != std::string::npos)
    {
      auto varName = pStr.substr(beginOfVarName, endVarPos - beginOfVarName);

      auto& variablesToValue = pProblem.variablesToValue();
      auto it = variablesToValue.find(varName);
      if (it != variablesToValue.end())
      {
        auto& varValue = it->second;
        pStr.replace(currentPos, endVarPos - currentPos + 1, varValue);
        currentPos += varValue.size();
      }
      else
      {
        currentPos = endVarPos;
      }

    }
    currentPos = pStr.find_first_of("${", currentPos);
  }

  // evalute expressions
  currentPos = pStr.find("`");
  while (currentPos != std::string::npos)
  {
    auto beginOfExp = currentPos + 1;
    auto endExpPos = pStr.find("`", beginOfExp);
    if (endExpPos == std::string::npos)
      break;
    pStr.replace(currentPos, endExpPos - currentPos + 1, evaluteToStr(pStr, beginOfExp));
    currentPos = endExpPos + 1;
    currentPos = pStr.find("`", currentPos);
  }
}



void fillReachableFacts(Problem& pProblem,
                        const Domain& pDomain)
{
  if (!pProblem.needToAddReachableFacts())
    return;
  for (const auto& currFact : pProblem.facts())
  {
    if (pProblem.reachableFacts().count(currFact) == 0)
      _feedReachableFacts(pProblem, currFact, pDomain);
  }
  _feedReachableFactsFromSetOfActions(pProblem, pDomain.actionsWithoutPrecondition(), pDomain);
  pProblem.noNeedToAddReachableFacts();
}


bool areFactsTrue(const SetOfFacts& pSetOfFacts,
                  const Problem& pProblem)
{
  std::map<std::string, std::string> parameters;
  return _areFactsTrue(parameters, pSetOfFacts, pProblem);
}


ActionId lookForAnActionToDo(std::map<std::string, std::string>& pParameters,
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    std::string* pGoalOfTheAction,
    const Historical* pGlobalHistorical)
{
  fillReachableFacts(pProblem, pDomain);

  ActionId res;
  auto tryToFindAnActionTowardGoal = [&](Goal& pGoal){
    auto& facts = pProblem.facts();
    auto* goalConditionFactPtr = pGoal.conditionFactPtr();
    if (goalConditionFactPtr == nullptr ||
        facts.count(*goalConditionFactPtr) > 0)
    {
      auto& goalFact = pGoal.fact();
      if (facts.count(goalFact) == 0)
      {
        res = _nextStepOfTheProblemForAGoal(pParameters, goalFact, pProblem,
                                            pDomain, pGlobalHistorical);
        if (!res.empty())
        {
          if (pGoalOfTheAction != nullptr)
            *pGoalOfTheAction = pGoal.toStr();
          pGoal.notifyActivity();
          return true;
        }
        return false;
      }
    }
    return false;
  };

  pProblem.iterateOnGoalAndRemoveNonPersistent(tryToFindAnActionTowardGoal, pNow);
  return res;
}


std::string printActionIdWithParameters(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters)
{
  std::string res = pActionId;
  if (!pParameters.empty())
  {
    res += "(";
    bool firstIeration = true;
    for (const auto& currParam : pParameters)
    {
      if (firstIeration)
        firstIeration = false;
      else
        res += ", ";
      res += currParam.first + " -> " + currParam.second;
    }
    res += ")";
  }
  return res;
}


std::list<ActionId> solve(Problem& pProblem,
                          const Domain& pDomain,
                          const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                          Historical* pGlobalHistorical)
{
  std::list<std::string> res;
  while (!pProblem.goals().empty())
  {
    std::map<std::string, std::string> parameters;
    auto actionToDo = lookForAnActionToDo(parameters, pProblem, pDomain, pNow, nullptr, pGlobalHistorical);
    if (actionToDo.empty())
      break;
    res.emplace_back(printActionIdWithParameters(actionToDo, parameters));

    auto itAction = pDomain.actions().find(actionToDo);
    if (itAction != pDomain.actions().end())
    {
      if (pGlobalHistorical != nullptr)
        pGlobalHistorical->notifyActionDone(actionToDo);
      pProblem.notifyActionDone(actionToDo, parameters, itAction->second.effects, pNow, nullptr);
    }
  }
  return res;
}


} // !cp
