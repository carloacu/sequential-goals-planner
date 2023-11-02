#include <contextualplanner/types/factmodification.hpp>
#include <sstream>
#include <contextualplanner/types/worldstate.hpp>
#include "expressionParsed.hpp"
#include <contextualplanner/util/util.hpp>

namespace cp
{
namespace
{
const std::string _setFunctionName = "set";
const std::string _forAllFunctionName = "forAll";
const std::string _addFunctionName = "add";

bool _areEqual(
    const std::unique_ptr<FactModification>& pCond1,
    const std::unique_ptr<FactModification>& pCond2)
{
  if (!pCond1 && !pCond2)
    return true;
  if (pCond1 && pCond2)
    return *pCond1 == *pCond2;
  return false;
}


std::unique_ptr<FactModification> _expressionParsedToFactModification(const ExpressionParsed& pExpressionParsed)
{
  std::unique_ptr<FactModification> res;

  if (pExpressionParsed.name == _setFunctionName &&
      pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<FactModificationNode>(FactModificationNodeType::SET,
                                              _expressionParsedToFactModification(pExpressionParsed.arguments.front()),
                                              _expressionParsedToFactModification(*(++pExpressionParsed.arguments.begin())));
  }
  else if (pExpressionParsed.name == _forAllFunctionName &&
      pExpressionParsed.arguments.size() == 3)
  {
    auto itArg = pExpressionParsed.arguments.begin();
    auto& firstArg = *itArg;
    ++itArg;
    auto& secondArg = *itArg;
    ++itArg;
    auto& thridArg = *itArg;
    res = std::make_unique<FactModificationNode>(FactModificationNodeType::FOR_ALL,
                                                 std::make_unique<FactModificationFact>(secondArg.toFact()),
                                                 _expressionParsedToFactModification(thridArg),
                                                 firstArg.name);
  }
  else if (pExpressionParsed.name == _addFunctionName &&
      pExpressionParsed.arguments.size() == 2)
  {
    auto itArg = pExpressionParsed.arguments.begin();
    auto& firstArg = *itArg;
    ++itArg;
    auto& secondArg = *itArg;
    std::unique_ptr<FactModification> rightOpPtr;
    try {
      rightOpPtr = std::make_unique<FactModificationNumber>(lexical_cast<int>(secondArg.name));
    }  catch (...) {}
    if (!rightOpPtr)
      rightOpPtr = _expressionParsedToFactModification(secondArg);

    res = std::make_unique<FactModificationNode>(FactModificationNodeType::ADD,
                                                 _expressionParsedToFactModification(firstArg),
                                                 std::move(rightOpPtr));
  }
  else
  {
    if (pExpressionParsed.arguments.empty() && pExpressionParsed.value == "")
    {
      try {
        res = std::make_unique<FactModificationNumber>(lexical_cast<int>(pExpressionParsed.name));
      }  catch (...) {}
    }

    if (!res)
      res = std::make_unique<FactModificationFact>(pExpressionParsed.toFact());
  }

  if (pExpressionParsed.followingExpression)
  {
    auto nodeType = FactModificationNodeType::AND;
    if (pExpressionParsed.separatorToFollowingExp == '+')
      nodeType = FactModificationNodeType::PLUS;
    else if (pExpressionParsed.separatorToFollowingExp == '-')
      nodeType = FactModificationNodeType::MINUS;
    res = std::make_unique<FactModificationNode>(nodeType,
                                                 std::move(res),
                                                 _expressionParsedToFactModification(*pExpressionParsed.followingExpression));
  }

  return res;
}

}


FactModification::FactModification(FactModificationType pType)
 : type(pType)
{
}


std::unique_ptr<FactModification> FactModification::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToFactModification(expressionParsed);
}

std::unique_ptr<FactModification> FactModification::merge(const FactModification& pFactModification1,
                                                          const FactModification& pFactModification2)
{
  return std::make_unique<FactModificationNode>(FactModificationNodeType::AND,
                                                pFactModification1.clone(nullptr),
                                                pFactModification2.clone(nullptr));
}


FactModificationNode::FactModificationNode(FactModificationNodeType pNodeType,
                                           std::unique_ptr<FactModification> pLeftOperand,
                                           std::unique_ptr<FactModification> pRightOperand,
                                           const std::string& pParameterName)
  : FactModification(FactModificationType::NODE),
   nodeType(pNodeType),
   leftOperand(std::move(pLeftOperand)),
   rightOperand(std::move(pRightOperand)),
   parameterName(pParameterName)
{
}

bool FactModificationNode::hasFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->hasFact(pFact)) ||
      (rightOperand && rightOperand->hasFact(pFact));
}

bool FactModificationNode::canModifySomethingInTheWorld() const
{
  if (nodeType == FactModificationNodeType::AND)
  {
    return (leftOperand && leftOperand->canModifySomethingInTheWorld()) ||
        (rightOperand && rightOperand->canModifySomethingInTheWorld());
  }

  if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
      return !leftFactPtr->factOptional.fact.isUnreachable();
  }

  if (nodeType == FactModificationNodeType::FOR_ALL)
    return rightOperand && rightOperand->canModifySomethingInTheWorld();

  if (nodeType == FactModificationNodeType::ADD)
    return true;

  return false;
}

bool FactModificationNode::isDynamic() const
{
  if (nodeType == FactModificationNodeType::SET ||
      nodeType == FactModificationNodeType::FOR_ALL ||
      nodeType == FactModificationNodeType::ADD)
    return true;
  return (leftOperand && leftOperand->isDynamic()) ||
      (rightOperand && rightOperand->isDynamic());
}


void FactModificationNode::replaceFact(const cp::Fact& pOldFact,
                                       const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}

void FactModificationNode::_forAllInstruction(const std::function<void (const FactModification&)>& pCallback,
                                              const WorldState& pWorldState) const
{
  if (leftOperand && rightOperand && !parameterName.empty())
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      std::set<cp::Fact> parameterValues;
      pWorldState.forAllInstruction(parameterName, leftFactPtr->factOptional.fact, parameterValues);
      if (!parameterValues.empty())
      {
        auto oldFact = Fact::fromStr(parameterName);
        for (const auto& paramValue : parameterValues)
        {
          auto newFactModif = rightOperand->clone(nullptr);
          newFactModif->replaceFact(oldFact, paramValue);
          pCallback(*newFactModif);
        }
      }
    }
  }
}

void FactModificationNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                                  const WorldState& pWorldState) const
{
  if (nodeType == FactModificationNodeType::AND)
  {
    if (leftOperand)
      leftOperand->forAll(pFactCallback, pWorldState);
    if (rightOperand)
      rightOperand->forAll(pFactCallback, pWorldState);
  }
  else if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      return pFactCallback(factToCheck);
    }

  }
  else if (nodeType == FactModificationNodeType::FOR_ALL)
  {
    _forAllInstruction(
          [&](const FactModification& pFactModification)
    {
      pFactModification.forAll(pFactCallback, pWorldState);
    }, pWorldState);
  }
  else if (nodeType == FactModificationNodeType::ADD && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }
}


bool FactModificationNode::forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                                           const WorldState& pWorldState) const
{
  if (nodeType == FactModificationNodeType::AND)
    return (leftOperand && leftOperand->forAllUntilTrue(pFactCallback, pWorldState)) ||
        (rightOperand && rightOperand->forAllUntilTrue(pFactCallback, pWorldState));

  if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      return pFactCallback(factToCheck);
    }
  }

  if (nodeType == FactModificationNodeType::FOR_ALL)
  {
    bool res = false;
    _forAllInstruction(
          [&](const FactModification& pFactModification)
    {
      if (!res)
        res = pFactModification.forAllUntilTrue(pFactCallback, pWorldState);
    }, pWorldState);
    return res;
  }

  if (nodeType == FactModificationNodeType::ADD && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }

  return false;
}

bool FactModificationNode::operator==(const FactModification& pOther) const
{
  auto* otherNodePtr = pOther.fmNodePtr();
  return otherNodePtr != nullptr &&
      nodeType == otherNodePtr->nodeType &&
      _areEqual(leftOperand, otherNodePtr->leftOperand) &&
      _areEqual(rightOperand, otherNodePtr->rightOperand) &&
      parameterName == otherNodePtr->parameterName;
}

std::string FactModificationNode::getValue(const WorldState& pWorldState) const
{
  if (nodeType == FactModificationNodeType::PLUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return plusIntOrStr(leftValue, rightValue);
  }
  if (nodeType == FactModificationNodeType::MINUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return minusIntOrStr(leftValue, rightValue);
  }
  return "";
}


std::unique_ptr<FactModification> FactModificationNode::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<FactModificationNode>(
        nodeType,
        leftOperand ? leftOperand->clone(pParametersPtr) : std::unique_ptr<FactModification>(),
        rightOperand ? rightOperand->clone(pParametersPtr) : std::unique_ptr<FactModification>(),
        parameterName);
}


std::unique_ptr<FactModification> FactModificationNode::cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const
{
  return std::make_unique<FactModificationNode>(
        nodeType,
        leftOperand ? leftOperand->cloneParamSet(pParameters) : std::unique_ptr<FactModification>(),
        rightOperand ? rightOperand->cloneParamSet(pParameters) : std::unique_ptr<FactModification>(),
        parameterName);
}

std::string FactModificationNode::toStr() const
{
  std::string leftOperandStr;
  if (leftOperand)
    leftOperandStr = leftOperand->toStr();
  std::string rightOperandStr;
  if (rightOperand)
    rightOperandStr = rightOperand->toStr();

  switch (nodeType)
  {
  case FactModificationNodeType::AND:
    return leftOperandStr + " & " + rightOperandStr;
  case FactModificationNodeType::SET:
    return _setFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case FactModificationNodeType::FOR_ALL:
    return _forAllFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case FactModificationNodeType::ADD:
    return _addFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case FactModificationNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case FactModificationNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  }
  return "";
}



FactModificationFact::FactModificationFact(const FactOptional& pFactOptional)
 : FactModification(FactModificationType::FACT),
   factOptional(pFactOptional)
{
}

bool FactModificationFact::hasFact(const cp::Fact& pFact) const
{
  return factOptional.fact == pFact;
}

bool FactModificationFact::canModifySomethingInTheWorld() const
{
  return factOptional.isFactNegated ||
      !factOptional.fact.isUnreachable();
}

void FactModificationFact::replaceFact(const cp::Fact& pOldFact,
                                       const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
  else
    factOptional.fact.replaceFactInArguments(pOldFact, pNewFact);
}

bool FactModificationFact::forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback, const WorldState&) const
{
  return pFactCallback(factOptional);
}

bool FactModificationFact::operator==(const FactModification& pOther) const
{
  auto* otherFactPtr = pOther.fcFactPtr();
  return otherFactPtr != nullptr &&
      factOptional == otherFactPtr->factOptional;
}

std::string FactModificationFact::getValue(const WorldState& pWorldState) const
{
  return pWorldState.getFactValue(factOptional.fact);
}

std::unique_ptr<FactModification> FactModificationFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<FactModificationFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.replaceArguments(*pParametersPtr);
  return res;
}


std::unique_ptr<FactModification> FactModificationFact::cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const
{
  auto res = std::make_unique<FactModificationFact>(factOptional);
  res->factOptional.fact.replaceArguments(pParameters);
  return res;
}



FactModificationNumber::FactModificationNumber(int pNb)
 : FactModification(FactModificationType::NUMBER),
   nb(pNb)
{
}

bool FactModificationNumber::operator==(const FactModification& pOther) const
{
  auto* otherNumberPtr = pOther.fcNumberPtr();
  return otherNumberPtr != nullptr &&
      nb == otherNumberPtr->nb;
}

std::string FactModificationNumber::getValue(const WorldState& pWorldState) const
{
  return toStr();
}


std::unique_ptr<FactModification> FactModificationNumber::clone(const std::map<std::string, std::string>*) const
{
  return std::make_unique<FactModificationNumber>(nb);
}

std::unique_ptr<FactModification> FactModificationNumber::cloneParamSet(const std::map<std::string, std::set<std::string>>&) const
{
  return std::make_unique<FactModificationNumber>(nb);
}

std::string FactModificationNumber::toStr() const
{
  std::stringstream ss;
  ss << nb;
  return ss.str();
}


} // !cp
