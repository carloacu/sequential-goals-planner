#include <contextualplanner/types/factcondition.hpp>
#include <sstream>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/util/util.hpp>
#include "expressionParsed.hpp"

namespace cp
{
namespace
{
const std::string _equalsFunctionName = "equals";


bool _forEachValueUntil(const std::function<bool (const std::string&)>& pValueCallback,
                        bool pUntilValue,
                        const FactCondition& pFactCondition,
                        const Problem& pProblem,
                        const std::map<std::string, std::set<std::string>>* pParametersPtr)
{
  if (pParametersPtr == nullptr || pParametersPtr->empty())
  {
    return pValueCallback(pFactCondition.getValue(pProblem));
  }

  std::list<std::map<std::string, std::string>> paramPossibilities;
  unfoldMapWithSet(paramPossibilities, *pParametersPtr);
  for (auto& currParamPoss : paramPossibilities)
  {
    auto factCondToExtractValue = pFactCondition.clone(&currParamPoss);
    if (pValueCallback(factCondToExtractValue->getValue(pProblem)) == pUntilValue)
      return pUntilValue;
  }
  return !pUntilValue;
}


void _forEach(const std::function<void (const std::string&)>& pValueCallback,
              const FactCondition& pFactCondition,
              const Problem& pProblem,
              const std::map<std::string, std::set<std::string>>* pParametersPtr)
{
  if (pParametersPtr == nullptr || pParametersPtr->empty())
  {
    pValueCallback(pFactCondition.getValue(pProblem));
    return;
  }

  std::list<std::map<std::string, std::string>> paramPossibilities;
  unfoldMapWithSet(paramPossibilities, *pParametersPtr);
  for (auto& currParamPoss : paramPossibilities)
  {
    auto factToExtractValue = pFactCondition.clone(&currParamPoss);
    pValueCallback(factToExtractValue->getValue(pProblem));
  }
}

bool _areEqual(
    const std::unique_ptr<FactCondition>& pCond1,
    const std::unique_ptr<FactCondition>& pCond2)
{
  if (!pCond1 && !pCond2)
    return true;
  if (pCond1 && pCond2)
    return *pCond1 == *pCond2;
  return false;
}


std::unique_ptr<FactCondition> _expressionParsedToFactCondition(const ExpressionParsed& pExpressionParsed)
{
  std::unique_ptr<FactCondition> res;

  if (pExpressionParsed.name == _equalsFunctionName &&
      pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<FactConditionNode>(FactConditionNodeType::EQUALITY,
                                              _expressionParsedToFactCondition(pExpressionParsed.arguments.front()),
                                              _expressionParsedToFactCondition(*(++pExpressionParsed.arguments.begin())));
  }
  else
  {
    if (pExpressionParsed.arguments.empty() && pExpressionParsed.value == "")
    {
      try {
        res = std::make_unique<FactConditionNumber>(lexical_cast<int>(pExpressionParsed.name));
      }  catch (...) {}
    }

    if (!res)
      res = std::make_unique<FactConditionFact>(pExpressionParsed.toFact());
  }

  if (pExpressionParsed.followingExpression)
  {
    auto nodeType = FactConditionNodeType::AND;
    if (pExpressionParsed.separatorToFollowingExp == '+')
      nodeType = FactConditionNodeType::PLUS;
    else if (pExpressionParsed.separatorToFollowingExp == '-')
      nodeType = FactConditionNodeType::MINUS;
    res = std::make_unique<FactConditionNode>(nodeType,
                                              std::move(res),
                                              _expressionParsedToFactCondition(*pExpressionParsed.followingExpression));
  }

  return res;
}


}


FactCondition::FactCondition(FactConditionType pType)
 : type(pType)
{
}


std::unique_ptr<FactCondition> FactCondition::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToFactCondition(expressionParsed);
}

FactConditionNode::FactConditionNode(FactConditionNodeType pNodeType,
                                     std::unique_ptr<FactCondition> pLeftOperand,
                                     std::unique_ptr<FactCondition> pRightOperand)
 : FactCondition(FactConditionType::NODE),
   nodeType(pNodeType),
   leftOperand(std::move(pLeftOperand)),
   rightOperand(std::move(pRightOperand))
{
}

bool FactConditionNode::hasFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->hasFact(pFact)) ||
      (rightOperand && rightOperand->hasFact(pFact));
}


bool FactConditionNode::containsFactOpt(const FactOptional& pFactOptional,
                                        const std::map<std::string, std::set<std::string>>& pFactParameters,
                                        const std::vector<std::string>& pThisFactParameters) const
{
  return (leftOperand && leftOperand->containsFactOpt(pFactOptional, pFactParameters, pThisFactParameters)) ||
      (rightOperand && rightOperand->containsFactOpt(pFactOptional, pFactParameters, pThisFactParameters));
}


void FactConditionNode::replaceFact(const cp::Fact& pOldFact,
                                    const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}


void FactConditionNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback) const
{
  if (leftOperand)
    leftOperand->forAll(pFactCallback);
  if (rightOperand)
    rightOperand->forAll(pFactCallback);
}

bool FactConditionNode::untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                                   const Problem& pProblem,
                                   const std::map<std::string, std::set<std::string>>& pParameters) const
{
  if (nodeType == FactConditionNodeType::AND)
  {
    if (leftOperand && !leftOperand->untilFalse(pFactCallback, pProblem, pParameters))
      return false;
    if (rightOperand && !rightOperand->untilFalse(pFactCallback, pProblem, pParameters))
      return false;
  }
  else if (nodeType == FactConditionNodeType::EQUALITY && leftOperand && rightOperand)
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
      }, false, *rightOperand, pProblem, &pParameters);
    }
  }
  return true;
}

bool FactConditionNode::canBeTrue() const
{
  if (leftOperand && !leftOperand->canBeTrue())
    return false;
  if (rightOperand && !rightOperand->canBeTrue())
    return false;
  return true;
}




bool FactConditionNode::isTrue(const Problem& pProblem,
                               const std::set<Fact>& pPunctualFacts,
                               const std::set<Fact>& pRemovedFacts,
                               std::map<std::string, std::set<std::string>>* pParametersPtr,
                               bool* pCanBecomeTruePtr) const
{
  if (nodeType == FactConditionNodeType::AND)
  {
    bool canBecomeTrue = false;
    if (pCanBecomeTruePtr == nullptr)
      pCanBecomeTruePtr = &canBecomeTrue;

    if (leftOperand && !leftOperand->isTrue(pProblem, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
    {
      // Sometimes for negation of fact with parameter we need to check in the inverse order
      if (pCanBecomeTruePtr != nullptr && *pCanBecomeTruePtr)
      {
        if (rightOperand && !rightOperand->isTrue(pProblem, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
          return false;

        if (leftOperand && !leftOperand->isTrue(pProblem, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
          return false;
        return true;
      }
      return false;
    }
    if (rightOperand && !rightOperand->isTrue(pProblem, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr))
      return false;
  }
  else if (nodeType == FactConditionNodeType::EQUALITY && leftOperand && rightOperand)
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
          res = factToCheck.isInFacts(pProblem._facts, true, newParameters, pParametersPtr) || res;
      }, *rightOperand, pProblem, pParametersPtr);

      if (pParametersPtr != nullptr)
        applyNewParams(*pParametersPtr, newParameters);
      return res;
    }
  }
  return true;
}

bool FactConditionNode::canBecomeTrue(const Problem& pProblem) const
{
  if (nodeType == FactConditionNodeType::AND)
  {
    if (leftOperand && !leftOperand->canBecomeTrue(pProblem))
      return false;
    if (rightOperand && !rightOperand->canBecomeTrue(pProblem))
      return false;
  }
  else if (nodeType == FactConditionNodeType::EQUALITY && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional.fact;
      factToCheck.value = pProblem.getFactValue(rightFactPtr->factOptional.fact);
      return pProblem.canFactBecomeTrue(factToCheck);
    }
  }
  return true;
}

bool FactConditionNode::operator==(const FactCondition& pOther) const
{
  auto* otherNodePtr = pOther.fcNodePtr();
  return otherNodePtr != nullptr &&
      nodeType == otherNodePtr->nodeType &&
      _areEqual(leftOperand, otherNodePtr->leftOperand) &&
      _areEqual(rightOperand, otherNodePtr->rightOperand);
}

std::string FactConditionNode::getValue(const Problem& pProblem) const
{
  if (nodeType == FactConditionNodeType::PLUS)
  {
    auto leftValue = leftOperand->getValue(pProblem);
    auto rightValue = rightOperand->getValue(pProblem);
    return plusIntOrStr(leftValue, rightValue);
  }
  if (nodeType == FactConditionNodeType::MINUS)
  {
    auto leftValue = leftOperand->getValue(pProblem);
    auto rightValue = rightOperand->getValue(pProblem);
    return minusIntOrStr(leftValue, rightValue);
  }
  return "";
}


std::unique_ptr<FactCondition> FactConditionNode::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<FactConditionNode>(
        nodeType,
        leftOperand ? leftOperand->clone(pParametersPtr) : std::unique_ptr<FactCondition>(),
        rightOperand ? rightOperand->clone(pParametersPtr) : std::unique_ptr<FactCondition>());
}

std::string FactConditionNode::toStr(const std::function<std::string (const Fact&)>* pFactWriterPtr) const
{
  std::string leftOperandStr;
  if (leftOperand)
    leftOperandStr = leftOperand->toStr(pFactWriterPtr);
  std::string rightOperandStr;
  if (rightOperand)
    rightOperandStr = rightOperand->toStr(pFactWriterPtr);

  switch (nodeType)
  {
  case FactConditionNodeType::AND:
    return leftOperandStr + " & " + rightOperandStr;
  case FactConditionNodeType::EQUALITY:
    return _equalsFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case FactConditionNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case FactConditionNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  }
  return "";
}



FactConditionFact::FactConditionFact(const FactOptional& pFactOptional)
 : FactCondition(FactConditionType::FACT),
   factOptional(pFactOptional)
{
}

bool FactConditionFact::hasFact(const cp::Fact& pFact) const
{
  return factOptional.fact == pFact;
}

bool FactConditionFact::containsFactOpt(const FactOptional& pFactOptional,
                                        const std::map<std::string, std::set<std::string>>& pFactParameters,
                                        const std::vector<std::string>& pThisFactParameters) const
{
  if (pFactOptional.isFactNegated == factOptional.isFactNegated)
    return factOptional.fact.areEqualExceptAnyValues(pFactOptional.fact, &pFactParameters, &pThisFactParameters);
  return false;
}

void FactConditionFact::replaceFact(const cp::Fact& pOldFact,
                                    const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
}

bool FactConditionFact::isTrue(const Problem& pProblem,
                               const std::set<Fact>& pPunctualFacts,
                               const std::set<Fact>& pRemovedFacts,
                               std::map<std::string, std::set<std::string>>* pParametersPtr,
                               bool* pCanBecomeTruePtr) const
{
  return pProblem.isFactPatternSatisfied(factOptional, pPunctualFacts, pRemovedFacts, pParametersPtr, pCanBecomeTruePtr);
}

bool FactConditionFact::canBecomeTrue(const Problem& pProblem) const
{
  if (factOptional.isFactNegated)
  {
    if (pProblem._removableFacts.count(factOptional.fact) == 0)
    {
      if (factOptional.fact.value == Fact::anyValue)
        return true;
      if (pProblem._facts.count(factOptional.fact) > 0)
        return false;
    }
    return true;
  }
  return pProblem.canFactBecomeTrue(factOptional.fact);
}

bool FactConditionFact::operator==(const FactCondition& pOther) const
{
  auto* otherFactPtr = pOther.fcFactPtr();
  return otherFactPtr != nullptr &&
      factOptional == otherFactPtr->factOptional;
}

std::string FactConditionFact::getValue(const Problem& pProblem) const
{
  return pProblem.getFactValue(factOptional.fact);
}

std::unique_ptr<FactCondition> FactConditionFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<FactConditionFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.fillParameters(*pParametersPtr);
  return res;
}




FactConditionNumber::FactConditionNumber(int pNb)
  : FactCondition(FactConditionType::NUMBER),
    nb(pNb)
{
}

bool FactConditionNumber::operator==(const FactCondition& pOther) const
{
  auto* otherNbPtr = pOther.fcNbPtr();
  return otherNbPtr != nullptr &&
      nb == otherNbPtr->nb;
}

std::string FactConditionNumber::getValue(const Problem&) const
{
  return toStr(nullptr);
}

std::unique_ptr<FactCondition> FactConditionNumber::clone(const std::map<std::string, std::string>*) const
{
  return std::make_unique<FactConditionNumber>(nb);
}

std::string FactConditionNumber::toStr(const std::function<std::string (const Fact&)>*) const
{
  std::stringstream ss;
  ss << nb;
  return ss.str();
}

} // !cp
