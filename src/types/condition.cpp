#include <contextualplanner/types/condition.hpp>
#include <optional>
#include <sstream>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/util/util.hpp>
#include "expressionParsed.hpp"

namespace cp
{
namespace
{
const char* _equalsFunctionName = "equals";
const char* _equalsCharFunctionName = "=";
const char* _existsFunctionName = "exists";
const char* _notFunctionName = "not";
const char* _superiorFunctionName = ">";
const char* _inferiorFunctionName = "<";
const char* _andFunctionName = "and";
const char* _orFunctionName = "or";


bool _forEachValueUntil(const std::function<bool (const std::string&)>& pValueCallback,
                        bool pUntilValue,
                        const Condition& pCondition,
                        const WorldState& pWorldState,
                        const std::map<std::string, std::set<std::string>>* pParametersPtr)
{
  if (pParametersPtr == nullptr || pParametersPtr->empty())
  {
    return pValueCallback(pCondition.getValue(pWorldState));
  }

  std::list<std::map<std::string, std::string>> paramPossibilities;
  unfoldMapWithSet(paramPossibilities, *pParametersPtr);
  for (auto& currParamPoss : paramPossibilities)
  {
    auto condToExtractValue = pCondition.clone(&currParamPoss);
    if (pValueCallback(condToExtractValue->getValue(pWorldState)) == pUntilValue)
      return pUntilValue;
  }
  return !pUntilValue;
}


void _forEach(const std::function<void (const std::string&)>& pValueCallback,
              const Condition& pCondition,
              const WorldState& pWorldState,
              const std::map<std::string, std::set<std::string>>* pParametersPtr)
{
  if (pParametersPtr == nullptr || pParametersPtr->empty())
  {
    pValueCallback(pCondition.getValue(pWorldState));
    return;
  }

  std::list<std::map<std::string, std::string>> paramPossibilities;
  unfoldMapWithSet(paramPossibilities, *pParametersPtr);
  for (auto& currParamPoss : paramPossibilities)
  {
    auto factToExtractValue = pCondition.clone(&currParamPoss);
    pValueCallback(factToExtractValue->getValue(pWorldState));
  }
}

bool _areEqual(
    const std::unique_ptr<Condition>& pCond1,
    const std::unique_ptr<Condition>& pCond2)
{
  if (!pCond1 && !pCond2)
    return true;
  if (pCond1 && pCond2)
    return *pCond1 == *pCond2;
  return false;
}


std::unique_ptr<Condition> _expressionParsedToCondition(const ExpressionParsed& pExpressionParsed)
{
  std::unique_ptr<Condition> res;

  if ((pExpressionParsed.name == _equalsFunctionName || pExpressionParsed.name == _equalsCharFunctionName) &&
      pExpressionParsed.arguments.size() == 2)
  {
    auto leftOperand = _expressionParsedToCondition(pExpressionParsed.arguments.front());
    const auto& rightOperandExp = *(++pExpressionParsed.arguments.begin());
    auto rightOperand = _expressionParsedToCondition(rightOperandExp);

    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr && !leftFactPtr->factOptional.isFactNegated)
    {
      auto* rightFactPtr = rightOperand->fcFactPtr();
      if (rightFactPtr != nullptr &&
          rightFactPtr->factOptional.fact.arguments.empty() &&
          rightFactPtr->factOptional.fact.value == "")
      {
        if (rightFactPtr->factOptional.fact.name == Fact::undefinedValue)
        {
          leftFactPtr->factOptional.isFactNegated = true;
          leftFactPtr->factOptional.fact.value = Fact::anyValue;
          res = std::make_unique<ConditionFact>(std::move(*leftFactPtr));
        }
        else if (pExpressionParsed.name == _equalsCharFunctionName && !rightOperandExp.isAFunction)
        {
          leftFactPtr->factOptional.fact.value = rightFactPtr->factOptional.fact.name;
          res = std::make_unique<ConditionFact>(std::move(*leftFactPtr));
        }
      }
    }

    if (!res)
      res = std::make_unique<ConditionNode>(ConditionNodeType::EQUALITY,
                                            std::move(leftOperand), std::move(rightOperand));
  }
  else if (pExpressionParsed.name == _existsFunctionName &&
           pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<ConditionExists>(pExpressionParsed.arguments.front().name,
                                            _expressionParsedToCondition(*(++pExpressionParsed.arguments.begin())));
  }
  else if (pExpressionParsed.name == _notFunctionName &&
           pExpressionParsed.arguments.size() == 1)
  {
    auto& expNegationned = pExpressionParsed.arguments.front();

    res = _expressionParsedToCondition(expNegationned);
    if (res)
    {
       auto* factPtr = res->fcFactPtr();
       if (factPtr != nullptr)
         factPtr->factOptional.isFactNegated = !factPtr->factOptional.isFactNegated;
       else
         res = std::make_unique<ConditionNot>(std::move(res));
    }
  }
  else if (pExpressionParsed.name == _superiorFunctionName &&
           pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<ConditionNode>(ConditionNodeType::SUPERIOR,
                                          _expressionParsedToCondition(pExpressionParsed.arguments.front()),
                                          _expressionParsedToCondition(*(++pExpressionParsed.arguments.begin())));
  }
  else if (pExpressionParsed.name == _inferiorFunctionName &&
           pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<ConditionNode>(ConditionNodeType::INFERIOR,
                                          _expressionParsedToCondition(pExpressionParsed.arguments.front()),
                                          _expressionParsedToCondition(*(++pExpressionParsed.arguments.begin())));
  }
  else if ((pExpressionParsed.name == _andFunctionName || pExpressionParsed.name == _orFunctionName) &&
           pExpressionParsed.arguments.size() >= 2)
  {
    auto listNodeType = pExpressionParsed.name == _andFunctionName ? ConditionNodeType::AND : ConditionNodeType::OR;
    std::list<std::unique_ptr<Condition>> elts;
    for (auto& currExp : pExpressionParsed.arguments)
      elts.emplace_back(_expressionParsedToCondition(currExp));

    res = std::make_unique<ConditionNode>(listNodeType, std::move(*(--(--elts.end()))), std::move(elts.back()));
    elts.pop_back();
    elts.pop_back();

    while (!elts.empty())
    {
      res = std::make_unique<ConditionNode>(listNodeType, std::move(elts.back()), std::move(res));
      elts.pop_back();
    }
  }
  else
  {
    if (pExpressionParsed.arguments.empty() && pExpressionParsed.value == "")
    {
      try {
        res = std::make_unique<ConditionNumber>(lexical_cast<int>(pExpressionParsed.name));
      }  catch (...) {}
    }

    if (!res)
      res = std::make_unique<ConditionFact>(pExpressionParsed.toFact());
  }

  if (pExpressionParsed.followingExpression)
  {
    auto nodeType = ConditionNodeType::AND;
    if (pExpressionParsed.separatorToFollowingExp == '+')
      nodeType = ConditionNodeType::PLUS;
    else if (pExpressionParsed.separatorToFollowingExp == '-')
      nodeType = ConditionNodeType::MINUS;
    else if (pExpressionParsed.separatorToFollowingExp == '>')
      nodeType = ConditionNodeType::SUPERIOR;
    else if (pExpressionParsed.separatorToFollowingExp == '<')
      nodeType = ConditionNodeType::INFERIOR;
    else if (pExpressionParsed.separatorToFollowingExp == '|')
      nodeType = ConditionNodeType::OR;
    res = std::make_unique<ConditionNode>(nodeType,
                                          std::move(res),
                                          _expressionParsedToCondition(*pExpressionParsed.followingExpression));
  }

  return res;
}


bool _existsIsTrueRec(std::map<std::string, std::set<std::string>>& pLocalParamToValue,
                      std::map<std::string, std::set<std::string>>* pConditionParametersToPossibleArguments,
                      const Condition& pCondition,
                      const std::set<Fact>& pFacts)
{
  auto* factOfConditionPtr = pCondition.fcFactPtr();
  if (factOfConditionPtr != nullptr)
  {
    bool res = false;
    std::map<std::string, std::set<std::string>> newParameters;

    const auto& factToOfCondition = factOfConditionPtr->factOptional.fact;
    res = factToOfCondition.isInOtherFacts(pFacts, true, &newParameters, pConditionParametersToPossibleArguments, &pLocalParamToValue) || res;

    if (pConditionParametersToPossibleArguments != nullptr)
      applyNewParams(*pConditionParametersToPossibleArguments, newParameters);
    return res;
  }

  auto* nodeOfConditionPtr = pCondition.fcNodePtr();
  if (nodeOfConditionPtr != nullptr &&
      nodeOfConditionPtr->leftOperand && nodeOfConditionPtr->rightOperand)
  {
    if (nodeOfConditionPtr->nodeType == ConditionNodeType::AND)
      return _existsIsTrueRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->leftOperand, pFacts) &&
          _existsIsTrueRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->rightOperand, pFacts);
    if (nodeOfConditionPtr->nodeType == ConditionNodeType::OR)
      return _existsIsTrueRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->leftOperand, pFacts) ||
          _existsIsTrueRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->rightOperand, pFacts);
  }
  return false;
}


void _existsExtractPossRec(std::map<std::string, std::set<std::string>>& pLocalParamToValue,
                           const std::map<std::string, std::set<std::string>>& pConditionParametersToPossibleArguments,
                           const Condition& pCondition,
                           const std::set<Fact>& pFacts,
                           const Fact& pFactFromEffect,
                           const std::string& pObject,
                           bool pIsNegated)
{
  auto* factOfConditionPtr = pCondition.fcFactPtr();
  if (factOfConditionPtr != nullptr)
  {
    const auto& factOfConditionOpt = factOfConditionPtr->factOptional;
    if (factOfConditionOpt.isFactNegated != pIsNegated ||
        !factOfConditionOpt.fact.areEqualWithoutAnArgConsideration(pFactFromEffect, pObject))
    {
      std::map<std::string, std::set<std::string>> newParameters;
      factOfConditionOpt.fact.isInOtherFacts(pFacts, true, &newParameters, &pConditionParametersToPossibleArguments, &pLocalParamToValue);
    }
    return;
  }

  auto* nodeOfConditionPtr = pCondition.fcNodePtr();
  if (nodeOfConditionPtr != nullptr &&
      nodeOfConditionPtr->leftOperand && nodeOfConditionPtr->rightOperand &&
      (nodeOfConditionPtr->nodeType == ConditionNodeType::AND || nodeOfConditionPtr->nodeType == ConditionNodeType::OR))
  {
    _existsExtractPossRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->leftOperand, pFacts, pFactFromEffect, pObject, pIsNegated);
    _existsExtractPossRec(pLocalParamToValue, pConditionParametersToPossibleArguments, *nodeOfConditionPtr->rightOperand, pFacts, pFactFromEffect, pObject, pIsNegated);
  }
}

}


std::unique_ptr<Condition> Condition::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToCondition(expressionParsed);
}

std::string ConditionNode::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr) const
{
  std::string leftOperandStr;
  if (leftOperand)
    leftOperandStr = leftOperand->toStr(pFactWriterPtr);
  std::string rightOperandStr;
  if (rightOperand)
    rightOperandStr = rightOperand->toStr(pFactWriterPtr);

  switch (nodeType)
  {
  case ConditionNodeType::AND:
    return leftOperandStr + " & " + rightOperandStr;
  case ConditionNodeType::OR:
    return leftOperandStr + " | " + rightOperandStr;
  case ConditionNodeType::EQUALITY:
    return std::string(_equalsFunctionName) + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case ConditionNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case ConditionNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  case ConditionNodeType::SUPERIOR:
    return leftOperandStr + ">" + rightOperandStr;
  case ConditionNodeType::INFERIOR:
    return leftOperandStr + "<" + rightOperandStr;
  }
  return "";
}


ConditionNode::ConditionNode(ConditionNodeType pNodeType,
                             std::unique_ptr<Condition> pLeftOperand,
                             std::unique_ptr<Condition> pRightOperand)
  : Condition(),
    nodeType(pNodeType),
    leftOperand(std::move(pLeftOperand)),
    rightOperand(std::move(pRightOperand))
{
}

bool ConditionNode::hasFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->hasFact(pFact)) ||
      (rightOperand && rightOperand->hasFact(pFact));
}


bool ConditionNode::containsFactOpt(const FactOptional& pFactOptional,
                                    const std::map<std::string, std::set<std::string>>& pFactParameters,
                                    const std::vector<std::string>& pConditionParameters,
                                    bool pIsWrappingExprssionNegated) const
{
  return (leftOperand && leftOperand->containsFactOpt(pFactOptional, pFactParameters, pConditionParameters, pIsWrappingExprssionNegated)) ||
      (rightOperand && rightOperand->containsFactOpt(pFactOptional, pFactParameters, pConditionParameters, pIsWrappingExprssionNegated));
}


void ConditionNode::replaceFact(const Fact& pOldFact,
                                const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}


void ConditionNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                           bool pIsWrappingExprssionNegated) const
{
  if (leftOperand)
    leftOperand->forAll(pFactCallback, pIsWrappingExprssionNegated);
  if (rightOperand)
    rightOperand->forAll(pFactCallback, pIsWrappingExprssionNegated);
}


bool ConditionNode::findConditionCandidateFromFactFromEffect(
    const std::function<bool (const FactOptional&)>& pDoesConditionFactMatchFactFromEffect,
    const WorldState& pWorldState,
    const Fact& pFactFromEffect,
    const std::map<std::string, std::set<std::string>>& pConditionParametersToPossibleArguments,
    bool pIsWrappingExprssionNegated) const
{
  if (nodeType == ConditionNodeType::AND || nodeType == ConditionNodeType::OR)
  {
    if (leftOperand && leftOperand->findConditionCandidateFromFactFromEffect(pDoesConditionFactMatchFactFromEffect, pWorldState, pFactFromEffect, pConditionParametersToPossibleArguments, pIsWrappingExprssionNegated))
      return true;
    if (rightOperand && rightOperand->findConditionCandidateFromFactFromEffect(pDoesConditionFactMatchFactFromEffect, pWorldState, pFactFromEffect, pConditionParametersToPossibleArguments, pIsWrappingExprssionNegated))
      return true;
  }
  else if (leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      const auto& leftFact = *leftFactPtr;
      if (nodeType == ConditionNodeType::EQUALITY)
      {
        return _forEachValueUntil(
              [&](const std::string& pValue)
        {
          auto factToCheck = leftFact.factOptional.fact;
          factToCheck.value = pValue;
          return pDoesConditionFactMatchFactFromEffect(FactOptional(factToCheck));
        }, true, *rightOperand, pWorldState, &pConditionParametersToPossibleArguments);
      }
      else if (nodeType == ConditionNodeType::SUPERIOR || nodeType == ConditionNodeType::INFERIOR)
      {
         return pDoesConditionFactMatchFactFromEffect(leftFact.factOptional);
      }
    }
  }
  return false;
}



bool ConditionNode::untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const WorldState& pWorldState) const
{
  if (nodeType == ConditionNodeType::AND || nodeType == ConditionNodeType::OR)
  {
    if (leftOperand && !leftOperand->untilFalse(pFactCallback, pWorldState))
      return false;
    if (rightOperand && !rightOperand->untilFalse(pFactCallback, pWorldState))
      return false;
  }
  else if (leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      const auto& leftFact = *leftFactPtr;
      if (nodeType == ConditionNodeType::EQUALITY)
      {
        auto factToCheck = leftFact.factOptional.fact;
        factToCheck.value = rightOperand->getValue(pWorldState);
        return pFactCallback(FactOptional(factToCheck));
      }
      else if (nodeType == ConditionNodeType::SUPERIOR || nodeType == ConditionNodeType::INFERIOR)
      {
         return pFactCallback(leftFact.factOptional);
      }
    }
  }
  return true;
}


bool ConditionNode::isTrue(const WorldState& pWorldState,
                           const std::set<Fact>& pPunctualFacts,
                           const std::set<Fact>& pRemovedFacts,
                           std::map<std::string, std::set<std::string>>* pConditionParametersToPossibleArguments,
                           bool* pCanBecomeTruePtr,
                           bool pIsWrappingExprssionNegated) const
{
  if (nodeType == ConditionNodeType::AND)
  {
    bool canBecomeTrue = false;
    if (pCanBecomeTruePtr == nullptr)
      pCanBecomeTruePtr = &canBecomeTrue;

    if (leftOperand && !leftOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
    {
      // Sometimes for negation of fact with parameter we need to check in the inverse order
      if (pCanBecomeTruePtr != nullptr && *pCanBecomeTruePtr)
      {
        if (rightOperand && !rightOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
          return false;

        if (leftOperand && !leftOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
          return false;
        return true;
      }
      return false;
    }
    if (rightOperand && !rightOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
      return false;
  }
  else if (nodeType == ConditionNodeType::OR)
  {
    bool canBecomeTrue = false;
    if (pCanBecomeTruePtr == nullptr)
      pCanBecomeTruePtr = &canBecomeTrue;

    if (leftOperand && leftOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
      return true;
    if (rightOperand && rightOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, pIsWrappingExprssionNegated))
      return true;
    return false;
  }
  else if (leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      const auto& leftFact = leftFactPtr->factOptional.fact;

      if (nodeType == ConditionNodeType::EQUALITY)
      {
        bool res = false;
        std::map<std::string, std::set<std::string>> newParameters;
        _forEach([&](const std::string& pValue)
        {
          auto factToCheck = leftFactPtr->factOptional.fact;
          factToCheck.value = pValue;
          const auto& facts = pWorldState.facts();
          if (factToCheck.isPunctual())
            res = pPunctualFacts.count(factToCheck) != 0 || res;
          else
            res = factToCheck.isInOtherFacts(facts, true, &newParameters, pConditionParametersToPossibleArguments) || res;
        }, *rightOperand, pWorldState, pConditionParametersToPossibleArguments);

        if (pConditionParametersToPossibleArguments != nullptr)
          applyNewParams(*pConditionParametersToPossibleArguments, newParameters);
        if (!pIsWrappingExprssionNegated)
          return res;
        return !res;
      }
      else if (nodeType == ConditionNodeType::SUPERIOR || nodeType == ConditionNodeType::INFERIOR)
      {
        auto* rightNbPtr = rightOperand->fcNbPtr();
        if (rightNbPtr != nullptr)
        {
          const auto& factNamesToFacts = pWorldState.factNamesToFacts();
          auto itWsFacts = factNamesToFacts.find(leftFact.name);
          if (itWsFacts != factNamesToFacts.end())
          {
            for (const auto& currWsFact : itWsFacts->second)
            {
              if (leftFact.areEqualWithoutValueConsideration(currWsFact))
              {
                bool res = compIntNb(currWsFact.value, rightNbPtr->nb, nodeType == ConditionNodeType::SUPERIOR);
                if (!pIsWrappingExprssionNegated)
                  return res;
                return !res;
              }
            }
          }
        }
      }
    }
  }
  return !pIsWrappingExprssionNegated;
}

bool ConditionNode::canBecomeTrue(const WorldState& pWorldState,
                                  bool pIsWrappingExprssionNegated) const
{
  if (nodeType == ConditionNodeType::AND)
  {
    if (leftOperand && !leftOperand->canBecomeTrue(pWorldState, pIsWrappingExprssionNegated))
      return pIsWrappingExprssionNegated;
    if (rightOperand && !rightOperand->canBecomeTrue(pWorldState, pIsWrappingExprssionNegated))
      return pIsWrappingExprssionNegated;
  }
  else if (nodeType == ConditionNodeType::OR)
  {
    if (leftOperand && leftOperand->canBecomeTrue(pWorldState, pIsWrappingExprssionNegated))
      return !pIsWrappingExprssionNegated;
    if (rightOperand && rightOperand->canBecomeTrue(pWorldState, pIsWrappingExprssionNegated))
      return !pIsWrappingExprssionNegated;
    return pIsWrappingExprssionNegated;
  }
  else if (nodeType == ConditionNodeType::EQUALITY && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional.fact;
      factToCheck.value = pWorldState.getFactValue(rightFactPtr->factOptional.fact);
      return pWorldState.canFactBecomeTrue(factToCheck);
    }
  }
  return true;
}

bool ConditionNode::operator==(const Condition& pOther) const
{
  auto* otherNodePtr = pOther.fcNodePtr();
  return otherNodePtr != nullptr &&
      nodeType == otherNodePtr->nodeType &&
      _areEqual(leftOperand, otherNodePtr->leftOperand) &&
      _areEqual(rightOperand, otherNodePtr->rightOperand);
}

std::string ConditionNode::getValue(const WorldState& pWorldState) const
{
  if (nodeType == ConditionNodeType::PLUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return plusIntOrStr(leftValue, rightValue);
  }
  if (nodeType == ConditionNodeType::MINUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return minusIntOrStr(leftValue, rightValue);
  }
  return "";
}


std::unique_ptr<Condition> ConditionNode::clone(const std::map<std::string, std::string>* pConditionParametersToArgumentPtr,
                                                bool pInvert) const
{
  if (!pInvert)
    return std::make_unique<ConditionNode>(
          nodeType,
          leftOperand ? leftOperand->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>(),
          rightOperand ? rightOperand->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>());

  std::optional<ConditionNodeType> invertedNodeOpt;
  switch (nodeType) {
  case ConditionNodeType::AND:
    invertedNodeOpt.emplace(ConditionNodeType::OR);
    break;
  case ConditionNodeType::OR:
    invertedNodeOpt.emplace(ConditionNodeType::AND);
    break;
  }
  if (invertedNodeOpt)
    return std::make_unique<ConditionNode>(
          *invertedNodeOpt,
          leftOperand ? leftOperand->clone(pConditionParametersToArgumentPtr, true) : std::unique_ptr<Condition>(),
          rightOperand ? rightOperand->clone(pConditionParametersToArgumentPtr, true) : std::unique_ptr<Condition>());

  return std::make_unique<ConditionNot>(
        std::make_unique<ConditionNode>(
          nodeType,
          leftOperand ? leftOperand->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>(),
          rightOperand ? rightOperand->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>()));
}




std::string ConditionExists::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr) const
{
  std::string conditionStr;
  if (condition)
    conditionStr = condition->toStr(pFactWriterPtr);
  return std::string(_existsFunctionName) + "(" + object + ", " + conditionStr + ")";
}

ConditionExists::ConditionExists(const std::string& pObject,
                                 std::unique_ptr<Condition> pCondition)
  : Condition(),
    object(pObject),
    condition(std::move(pCondition))
{
}

bool ConditionExists::hasFact(const Fact& pFact) const
{
  return condition && condition->hasFact(pFact);
}


bool ConditionExists::containsFactOpt(const FactOptional& pFactOptional,
                                      const std::map<std::string, std::set<std::string>>& pFactParameters,
                                      const std::vector<std::string>& pConditionParameters,
                                      bool pIsWrappingExprssionNegated) const
{
  return condition && condition->containsFactOpt(pFactOptional, pFactParameters, pConditionParameters, pIsWrappingExprssionNegated);
}

void ConditionExists::replaceFact(const Fact& pOldFact,
                                  const Fact& pNewFact)
{
  if (condition)
    condition->replaceFact(pOldFact, pNewFact);
}

void ConditionExists::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                             bool pIsWrappingExprssionNegated) const
{

  if (condition)
    condition->forAll(pFactCallback, pIsWrappingExprssionNegated);
}


bool ConditionExists::findConditionCandidateFromFactFromEffect(
    const std::function<bool (const FactOptional&)>& pDoesConditionFactMatchFactFromEffect,
    const WorldState& pWorldState,
    const Fact& pFactFromEffect,
    const std::map<std::string, std::set<std::string>>& pConditionParametersToPossibleArguments,
    bool pIsWrappingExprssionNegated) const
{
  if (condition)
  {
    const auto& facts = pWorldState.facts();
    std::map<std::string, std::set<std::string>> localParamToValue{{object, {}}};
    _existsExtractPossRec(localParamToValue, pConditionParametersToPossibleArguments, *condition, facts, pFactFromEffect, object, pIsWrappingExprssionNegated);

    return condition->findConditionCandidateFromFactFromEffect([&](const FactOptional& pConditionFact) {
      auto factToConsider = pConditionFact.fact;
      factToConsider.replaceArguments(localParamToValue);
      return pDoesConditionFactMatchFactFromEffect(FactOptional(factToConsider)) == !pIsWrappingExprssionNegated;
    }, pWorldState, pFactFromEffect, pConditionParametersToPossibleArguments, pIsWrappingExprssionNegated);
  }
  return pIsWrappingExprssionNegated;
}


bool ConditionExists::isTrue(const WorldState& pWorldState,
                             const std::set<Fact>& pPunctualFacts,
                             const std::set<Fact>& pRemovedFacts,
                             std::map<std::string, std::set<std::string>>* pConditionParametersToPossibleArguments,
                             bool* pCanBecomeTruePtr,
                             bool pIsWrappingExprssionNegated) const
{
  if (condition)
  {
    const auto& facts = pWorldState.facts();
    std::map<std::string, std::set<std::string>> localParamToValue{{object, {}}};
    return _existsIsTrueRec(localParamToValue, pConditionParametersToPossibleArguments, *condition, facts) == !pIsWrappingExprssionNegated;
  }
  return !pIsWrappingExprssionNegated;
}


bool ConditionExists::canBecomeTrue(const WorldState& pWorldState,
                                    bool pIsWrappingExprssionNegated) const
{
  if (condition)
  {
    auto* factOfConditionPtr = condition->fcFactPtr();
    if (factOfConditionPtr != nullptr)
    {
      const auto& factToOfCondition = factOfConditionPtr->factOptional.fact;
      std::set<Fact> potentialArgumentsOfTheParameter;
      pWorldState.extractPotentialArgumentsOfAFactParameter(potentialArgumentsOfTheParameter,
                                                            factToOfCondition, object);
      for (auto& currPot : potentialArgumentsOfTheParameter)
      {
        if (currPot.arguments.empty() && currPot.value == "")
        {
          auto factToCheck = factToOfCondition;
          factToCheck.replaceArguments({{object, currPot.name}});
          if (pWorldState.canFactBecomeTrue(factToCheck))
            return true;
        }
      }
      return pIsWrappingExprssionNegated;
    }
  }
  return true;
}

bool ConditionExists::operator==(const Condition& pOther) const
{
  auto* otherExistsPtr = pOther.fcExistsPtr();
  return otherExistsPtr != nullptr &&
      object == otherExistsPtr->object &&
      _areEqual(condition, otherExistsPtr->condition);
}

std::unique_ptr<Condition> ConditionExists::clone(const std::map<std::string, std::string>* pConditionParametersToArgumentPtr,
                                                  bool pInvert) const
{
  auto res = std::make_unique<ConditionExists>(
        object,
        condition ? condition->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>());
  if (pInvert)
    return std::make_unique<ConditionNot>(std::move(res));
  return res;
}




std::string ConditionNot::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr) const
{
  std::string conditionStr;
  if (condition)
    conditionStr = condition->toStr(pFactWriterPtr);
  if (condition->fcExistsPtr() != nullptr)
    return "!" + conditionStr;
  return "!(" + conditionStr + ")";
}

ConditionNot::ConditionNot(std::unique_ptr<Condition> pCondition)
  : Condition(),
    condition(std::move(pCondition))
{
}

bool ConditionNot::hasFact(const Fact& pFact) const
{
  return condition && condition->hasFact(pFact);
}


bool ConditionNot::containsFactOpt(const FactOptional& pFactOptional,
                                   const std::map<std::string, std::set<std::string>>& pFactParameters,
                                   const std::vector<std::string>& pConditionParameters,
                                   bool pIsWrappingExprssionNegated) const
{
  return condition && condition->containsFactOpt(pFactOptional, pFactParameters, pConditionParameters, !pIsWrappingExprssionNegated);
}

void ConditionNot::replaceFact(const Fact& pOldFact,
                               const Fact& pNewFact)
{
  if (condition)
    condition->replaceFact(pOldFact, pNewFact);
}

void ConditionNot::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                          bool pIsWrappingExprssionNegated) const
{

  if (condition)
    condition->forAll(pFactCallback, !pIsWrappingExprssionNegated);
}


bool ConditionNot::findConditionCandidateFromFactFromEffect(
    const std::function<bool (const FactOptional&)>& pDoesConditionFactMatchFactFromEffect,
    const WorldState& pWorldState,
    const Fact& pFactFromEffect,
    const std::map<std::string, std::set<std::string>>& pConditionParametersToPossibleArguments,
    bool pIsWrappingExprssionNegated) const
{
  if (condition)
    return condition->findConditionCandidateFromFactFromEffect(pDoesConditionFactMatchFactFromEffect, pWorldState, pFactFromEffect,
                                                               pConditionParametersToPossibleArguments, !pIsWrappingExprssionNegated);
  return pIsWrappingExprssionNegated;
}


bool ConditionNot::isTrue(const WorldState& pWorldState,
                          const std::set<Fact>& pPunctualFacts,
                          const std::set<Fact>& pRemovedFacts,
                          std::map<std::string, std::set<std::string>>* pConditionParametersToPossibleArguments,
                          bool* pCanBecomeTruePtr,
                          bool pIsWrappingExprssionNegated) const
{
  if (condition)
    return condition->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr, !pIsWrappingExprssionNegated);
  return !pIsWrappingExprssionNegated;
}


bool ConditionNot::canBecomeTrue(const WorldState& pWorldState,
                                 bool pIsWrappingExprssionNegated) const
{
  if (condition)
    return condition->canBecomeTrue(pWorldState, !pIsWrappingExprssionNegated);
  return true;
}

bool ConditionNot::operator==(const Condition& pOther) const
{
  auto* otherNotPtr = pOther.fcNotPtr();
  return otherNotPtr != nullptr &&
      _areEqual(condition, otherNotPtr->condition);
}

std::unique_ptr<Condition> ConditionNot::clone(const std::map<std::string, std::string>* pConditionParametersToArgumentPtr,
                                               bool pInvert) const
{
  if (pInvert)
    return condition ? condition->clone(pConditionParametersToArgumentPtr, false) : std::unique_ptr<Condition>();
  return std::make_unique<ConditionNot>(condition ? condition->clone(pConditionParametersToArgumentPtr, pInvert) : std::unique_ptr<Condition>());
}






ConditionFact::ConditionFact(const FactOptional& pFactOptional)
  : Condition(),
    factOptional(pFactOptional)
{
}

bool ConditionFact::hasFact(const Fact& pFact) const
{
  return factOptional.fact == pFact;
}

bool ConditionFact::containsFactOpt(const FactOptional& pFactOptional,
                                    const std::map<std::string, std::set<std::string>>& pFactParameters,
                                    const std::vector<std::string>& pConditionParameters,
                                    bool pIsWrappingExprssionNegated) const
{
  if ((!pIsWrappingExprssionNegated && pFactOptional.isFactNegated == factOptional.isFactNegated) ||
      (pIsWrappingExprssionNegated && pFactOptional.isFactNegated != factOptional.isFactNegated))
    return factOptional.fact.areEqualExceptAnyValues(pFactOptional.fact, &pFactParameters, &pConditionParameters);
  return false;
}

void ConditionFact::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                           bool pIsWrappingExprssionNegated) const
{
  if (!pIsWrappingExprssionNegated)
  {
    pFactCallback(factOptional);
  }
  else
  {
    auto factOptionalCopied = factOptional;
    factOptionalCopied.isFactNegated = !factOptionalCopied.isFactNegated;
    pFactCallback(factOptionalCopied);
  }
}


bool ConditionFact::findConditionCandidateFromFactFromEffect(
    const std::function<bool (const FactOptional&)>& pDoesConditionFactMatchFactFromEffect,
    const WorldState&,
    const Fact&,
    const std::map<std::string, std::set<std::string>>&,
    bool pIsWrappingExprssionNegated) const
{
  bool res = pDoesConditionFactMatchFactFromEffect(factOptional);
  if (pIsWrappingExprssionNegated)
    return !res;
  return res;
}




void ConditionFact::replaceFact(const Fact& pOldFact,
                                const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
}

bool ConditionFact::isTrue(const WorldState& pWorldState,
                           const std::set<Fact>& pPunctualFacts,
                           const std::set<Fact>& pRemovedFacts,
                           std::map<std::string, std::set<std::string>>* pConditionParametersToPossibleArguments,
                           bool* pCanBecomeTruePtr,
                           bool pIsWrappingExprssionNegated) const
{
  bool res = pWorldState.isOptionalFactSatisfiedInASpecificContext(factOptional, pPunctualFacts, pRemovedFacts, pConditionParametersToPossibleArguments, pCanBecomeTruePtr);
  if (!pIsWrappingExprssionNegated)
    return res;
  return !res;
}

bool ConditionFact::canBecomeTrue(const WorldState& pWorldState,
                                  bool pIsWrappingExprssionNegated) const
{
  bool res =  pWorldState.canFactOptBecomeTrue(factOptional);
  if (!pIsWrappingExprssionNegated)
    return res;
  return !res;
}

bool ConditionFact::operator==(const Condition& pOther) const
{
  auto* otherFactPtr = pOther.fcFactPtr();
  return otherFactPtr != nullptr &&
      factOptional == otherFactPtr->factOptional;
}

std::string ConditionFact::getValue(const WorldState& pWorldState) const
{
  return pWorldState.getFactValue(factOptional.fact);
}

std::unique_ptr<Condition> ConditionFact::clone(const std::map<std::string, std::string>* pConditionParametersToArgumentPtr,
                                                bool pInvert) const
{
  auto res = std::make_unique<ConditionFact>(factOptional);
  if (pConditionParametersToArgumentPtr != nullptr)
    res->factOptional.fact.replaceArguments(*pConditionParametersToArgumentPtr);
  if (pInvert)
    res->factOptional.isFactNegated = !res->factOptional.isFactNegated;
  return res;
}


std::string ConditionNumber::toStr(const std::function<std::string (const Fact&)>*) const
{
  std::stringstream ss;
  ss << nb;
  return ss.str();
}

ConditionNumber::ConditionNumber(int pNb)
  : Condition(),
    nb(pNb)
{
}

bool ConditionNumber::operator==(const Condition& pOther) const
{
  auto* otherNbPtr = pOther.fcNbPtr();
  return otherNbPtr != nullptr &&
      nb == otherNbPtr->nb;
}

std::string ConditionNumber::getValue(const WorldState& pWorldState) const
{
  return toStr(nullptr);
}

std::unique_ptr<Condition> ConditionNumber::clone(const std::map<std::string, std::string>*,
                                                  bool) const
{
  return std::make_unique<ConditionNumber>(nb);
}


} // !cp
