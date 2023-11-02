#include <contextualplanner/types/condition.hpp>
#include <sstream>
#include <contextualplanner/types/worldstate.hpp>
#include <contextualplanner/util/util.hpp>
#include "expressionParsed.hpp"

namespace cp
{
namespace
{
const std::string _equalsFunctionName = "equals";


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

  if (pExpressionParsed.name == _equalsFunctionName &&
      pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<ConditionNode>(ConditionNodeType::EQUALITY,
                                          _expressionParsedToCondition(pExpressionParsed.arguments.front()),
                                          _expressionParsedToCondition(*(++pExpressionParsed.arguments.begin())));
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
    res = std::make_unique<ConditionNode>(nodeType,
                                          std::move(res),
                                          _expressionParsedToCondition(*pExpressionParsed.followingExpression));
  }

  return res;
}


}


Condition::Condition(ConditionType pType)
  : type(pType)
{
}


std::unique_ptr<Condition> Condition::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToCondition(expressionParsed);
}

ConditionNode::ConditionNode(ConditionNodeType pNodeType,
                             std::unique_ptr<Condition> pLeftOperand,
                             std::unique_ptr<Condition> pRightOperand)
  : Condition(ConditionType::NODE),
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
                                    const std::vector<std::string>& pThisFactParameters) const
{
  return (leftOperand && leftOperand->containsFactOpt(pFactOptional, pFactParameters, pThisFactParameters)) ||
      (rightOperand && rightOperand->containsFactOpt(pFactOptional, pFactParameters, pThisFactParameters));
}


void ConditionNode::replaceFact(const cp::Fact& pOldFact,
                                const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}


void ConditionNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback) const
{
  if (leftOperand)
    leftOperand->forAll(pFactCallback);
  if (rightOperand)
    rightOperand->forAll(pFactCallback);
}

bool ConditionNode::untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                               const WorldState& pWorldState,
                               const std::map<std::string, std::set<std::string>>& pParameters) const
{
  if (nodeType == ConditionNodeType::AND)
  {
    if (leftOperand && !leftOperand->untilFalse(pFactCallback, pWorldState, pParameters))
      return false;
    if (rightOperand && !rightOperand->untilFalse(pFactCallback, pWorldState, pParameters))
      return false;
  }
  else if (nodeType == ConditionNodeType::EQUALITY && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      return _forEachValueUntil(
            [&](const std::string& pValue)
      {
        auto factToCheck = leftFactPtr->factOptional.fact;
        factToCheck.value = pValue;
        return pFactCallback(FactOptional(factToCheck));
      }, false, *rightOperand, pWorldState, &pParameters);
    }
  }
  return true;
}

bool ConditionNode::canBeTrue() const
{
  if (leftOperand && !leftOperand->canBeTrue())
    return false;
  if (rightOperand && !rightOperand->canBeTrue())
    return false;
  return true;
}




bool ConditionNode::isTrue(const WorldState& pWorldState,
                           const std::set<Fact>& pPunctualFacts,
                           const std::set<Fact>& pRemovedFacts,
                           std::map<std::string, std::set<std::string>>* pParametersPtr,
                           bool* pCanBecomeTruePtr) const
{
  if (nodeType == ConditionNodeType::AND)
  {
    bool canBecomeTrue = false;
    if (pCanBecomeTruePtr == nullptr)
      pCanBecomeTruePtr = &canBecomeTrue;

    if (leftOperand && !leftOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
    {
      // Sometimes for negation of fact with parameter we need to check in the inverse order
      if (pCanBecomeTruePtr != nullptr && *pCanBecomeTruePtr)
      {
        if (rightOperand && !rightOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
          return false;

        if (leftOperand && !leftOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
          return false;
        return true;
      }
      return false;
    }
    if (rightOperand && !rightOperand->isTrue(pWorldState, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
      return false;
  }
  else if (nodeType == ConditionNodeType::EQUALITY && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      bool res = false;
      std::map<std::string, std::set<std::string>> newParameters;
      _forEach(
            [&](const std::string& pValue)
      {
        auto factToCheck = leftFactPtr->factOptional.fact;
        factToCheck.value = pValue;
        if (factToCheck.isPunctual())
          res = pPunctualFacts.count(factToCheck) != 0 || res;
        else
          res = factToCheck.isInOtherFacts(pWorldState._facts, true, &newParameters, pParametersPtr) || res;
      }, *rightOperand, pWorldState, pParametersPtr);

      if (pParametersPtr != nullptr)
        applyNewParams(*pParametersPtr, newParameters);
      return res;
    }
  }
  return true;
}

bool ConditionNode::canBecomeTrue(const WorldState& pWorldState) const
{
  if (nodeType == ConditionNodeType::AND)
  {
    if (leftOperand && !leftOperand->canBecomeTrue(pWorldState))
      return false;
    if (rightOperand && !rightOperand->canBecomeTrue(pWorldState))
      return false;
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


std::unique_ptr<Condition> ConditionNode::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<ConditionNode>(
        nodeType,
        leftOperand ? leftOperand->clone(pParametersPtr) : std::unique_ptr<Condition>(),
        rightOperand ? rightOperand->clone(pParametersPtr) : std::unique_ptr<Condition>());
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
  case ConditionNodeType::EQUALITY:
    return _equalsFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case ConditionNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case ConditionNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  }
  return "";
}



ConditionFact::ConditionFact(const FactOptional& pFactOptional)
  : Condition(ConditionType::FACT),
    factOptional(pFactOptional)
{
}

bool ConditionFact::hasFact(const cp::Fact& pFact) const
{
  return factOptional.fact == pFact;
}

bool ConditionFact::containsFactOpt(const FactOptional& pFactOptional,
                                    const std::map<std::string, std::set<std::string>>& pFactParameters,
                                    const std::vector<std::string>& pThisFactParameters) const
{
  if (pFactOptional.isFactNegated == factOptional.isFactNegated)
    return factOptional.fact.areEqualExceptAnyValues(pFactOptional.fact, &pFactParameters, &pThisFactParameters);
  return false;
}

void ConditionFact::replaceFact(const cp::Fact& pOldFact,
                                const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
}

bool ConditionFact::isTrue(const WorldState& pWorldState,
                           const std::set<Fact>& pPunctualFacts,
                           const std::set<Fact>& pRemovedFacts,
                           std::map<std::string, std::set<std::string>>* pParametersPtr,
                           bool* pCanBecomeTruePtr) const
{
  return pWorldState.isFactPatternSatisfied(factOptional, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr);
}

bool ConditionFact::canBecomeTrue(const WorldState& pWorldState) const
{
  if (factOptional.isFactNegated)
  {
    if (pWorldState._removableFacts.count(factOptional.fact) == 0)
    {
      if (factOptional.fact.value == Fact::anyValue)
        return true;
      if (pWorldState._facts.count(factOptional.fact) > 0)
        return false;
    }
    return true;
  }
  return pWorldState.canFactBecomeTrue(factOptional.fact);
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

std::unique_ptr<Condition> ConditionFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<ConditionFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.replaceArguments(*pParametersPtr);
  return res;
}




ConditionNumber::ConditionNumber(int pNb)
  : Condition(ConditionType::NUMBER),
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

std::unique_ptr<Condition> ConditionNumber::clone(const std::map<std::string, std::string>*) const
{
  return std::make_unique<ConditionNumber>(nb);
}

std::string ConditionNumber::toStr(const std::function<std::string (const Fact&)>*) const
{
  std::stringstream ss;
  ss << nb;
  return ss.str();
}

} // !cp
