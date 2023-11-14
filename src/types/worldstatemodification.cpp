#include <contextualplanner/types/worldstatemodification.hpp>
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
    const std::unique_ptr<WorldStateModification>& pCond1,
    const std::unique_ptr<WorldStateModification>& pCond2)
{
  if (!pCond1 && !pCond2)
    return true;
  if (pCond1 && pCond2)
    return *pCond1 == *pCond2;
  return false;
}


std::unique_ptr<WorldStateModification> _expressionParsedToWsModification(const ExpressionParsed& pExpressionParsed)
{
  std::unique_ptr<WorldStateModification> res;

  if (pExpressionParsed.name == _setFunctionName &&
      pExpressionParsed.arguments.size() == 2)
  {
    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::SET,
                                                       _expressionParsedToWsModification(pExpressionParsed.arguments.front()),
                                                       _expressionParsedToWsModification(*(++pExpressionParsed.arguments.begin())));
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
    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::FOR_ALL,
                                                       std::make_unique<WorldStateModificationFact>(secondArg.toFact()),
                                                       _expressionParsedToWsModification(thridArg),
                                                       firstArg.name);
  }
  else if (pExpressionParsed.name == _addFunctionName &&
           pExpressionParsed.arguments.size() == 2)
  {
    auto itArg = pExpressionParsed.arguments.begin();
    auto& firstArg = *itArg;
    ++itArg;
    auto& secondArg = *itArg;
    std::unique_ptr<WorldStateModification> rightOpPtr;
    try {
      rightOpPtr = std::make_unique<WorldStateModificationNumber>(lexical_cast<int>(secondArg.name));
    }  catch (...) {}
    if (!rightOpPtr)
      rightOpPtr = _expressionParsedToWsModification(secondArg);

    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::ADD,
                                                       _expressionParsedToWsModification(firstArg),
                                                       std::move(rightOpPtr));
  }
  else
  {
    if (pExpressionParsed.arguments.empty() && pExpressionParsed.value == "")
    {
      try {
        res = std::make_unique<WorldStateModificationNumber>(lexical_cast<int>(pExpressionParsed.name));
      }  catch (...) {}
    }

    if (!res)
      res = std::make_unique<WorldStateModificationFact>(pExpressionParsed.toFact());
  }

  if (pExpressionParsed.followingExpression)
  {
    auto nodeType = WorldStateModificationNodeType::AND;
    if (pExpressionParsed.separatorToFollowingExp == '+')
      nodeType = WorldStateModificationNodeType::PLUS;
    else if (pExpressionParsed.separatorToFollowingExp == '-')
      nodeType = WorldStateModificationNodeType::MINUS;
    res = std::make_unique<WorldStateModificationNode>(nodeType,
                                                       std::move(res),
                                                       _expressionParsedToWsModification(*pExpressionParsed.followingExpression));
  }

  return res;
}

}


WorldStateModification::WorldStateModification(WorldStateModificationType pType)
  : type(pType)
{
}


std::unique_ptr<WorldStateModification> WorldStateModification::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToWsModification(expressionParsed);
}

std::unique_ptr<WorldStateModification> WorldStateModification::merge(const WorldStateModification& pWsModif1,
                                                                      const WorldStateModification& pWsModif2)
{
  return std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::AND,
                                                      pWsModif1.clone(nullptr),
                                                      pWsModif2.clone(nullptr));
}


WorldStateModificationNode::WorldStateModificationNode(WorldStateModificationNodeType pNodeType,
                                                       std::unique_ptr<WorldStateModification> pLeftOperand,
                                                       std::unique_ptr<WorldStateModification> pRightOperand,
                                                       const std::string& pParameterName)
  : WorldStateModification(WorldStateModificationType::NODE),
    nodeType(pNodeType),
    leftOperand(std::move(pLeftOperand)),
    rightOperand(std::move(pRightOperand)),
    parameterName(pParameterName)
{
}

bool WorldStateModificationNode::hasFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->hasFact(pFact)) ||
      (rightOperand && rightOperand->hasFact(pFact));
}

bool WorldStateModificationNode::canModifySomethingInTheWorld() const
{
  if (nodeType == WorldStateModificationNodeType::AND)
  {
    return (leftOperand && leftOperand->canModifySomethingInTheWorld()) ||
        (rightOperand && rightOperand->canModifySomethingInTheWorld());
  }

  if (nodeType == WorldStateModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
      return !leftFactPtr->factOptional.fact.isUnreachable();
  }

  if (nodeType == WorldStateModificationNodeType::FOR_ALL)
    return rightOperand && rightOperand->canModifySomethingInTheWorld();

  if (nodeType == WorldStateModificationNodeType::ADD)
    return true;

  return false;
}

bool WorldStateModificationNode::isDynamic() const
{
  if (nodeType == WorldStateModificationNodeType::SET ||
      nodeType == WorldStateModificationNodeType::FOR_ALL ||
      nodeType == WorldStateModificationNodeType::ADD)
    return true;
  return (leftOperand && leftOperand->isDynamic()) ||
      (rightOperand && rightOperand->isDynamic());
}


void WorldStateModificationNode::replaceFact(const cp::Fact& pOldFact,
                                             const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}

void WorldStateModificationNode::_forAllInstruction(const std::function<void (const WorldStateModification &)> &pCallback,
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
          auto newWsModif = rightOperand->clone(nullptr);
          newWsModif->replaceFact(oldFact, paramValue);
          pCallback(*newWsModif);
        }
      }
    }
  }
}

void WorldStateModificationNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                                        const WorldState& pWorldState) const
{
  if (nodeType == WorldStateModificationNodeType::AND)
  {
    if (leftOperand)
      leftOperand->forAll(pFactCallback, pWorldState);
    if (rightOperand)
      rightOperand->forAll(pFactCallback, pWorldState);
  }
  else if (nodeType == WorldStateModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      return pFactCallback(factToCheck);
    }

  }
  else if (nodeType == WorldStateModificationNodeType::FOR_ALL)
  {
    _forAllInstruction(
          [&](const WorldStateModification& pWsModification)
    {
      pWsModification.forAll(pFactCallback, pWorldState);
    }, pWorldState);
  }
  else if (nodeType == WorldStateModificationNodeType::ADD && leftOperand && rightOperand)
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


bool WorldStateModificationNode::forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                                                 const WorldState& pWorldState) const
{
  if (nodeType == WorldStateModificationNodeType::AND)
    return (leftOperand && leftOperand->forAllUntilTrue(pFactCallback, pWorldState)) ||
        (rightOperand && rightOperand->forAllUntilTrue(pFactCallback, pWorldState));

  if (nodeType == WorldStateModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      return pFactCallback(factToCheck);
    }
  }

  if (nodeType == WorldStateModificationNodeType::FOR_ALL)
  {
    bool res = false;
    _forAllInstruction(
          [&](const WorldStateModification& pWsModification)
    {
      if (!res)
        res = pWsModification.forAllUntilTrue(pFactCallback, pWorldState);
    }, pWorldState);
    return res;
  }

  if (nodeType == WorldStateModificationNodeType::ADD && leftOperand && rightOperand)
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

bool WorldStateModificationNode::operator==(const WorldStateModification& pOther) const
{
  auto* otherNodePtr = pOther.fmNodePtr();
  return otherNodePtr != nullptr &&
      nodeType == otherNodePtr->nodeType &&
      _areEqual(leftOperand, otherNodePtr->leftOperand) &&
      _areEqual(rightOperand, otherNodePtr->rightOperand) &&
      parameterName == otherNodePtr->parameterName;
}

std::string WorldStateModificationNode::getValue(const WorldState& pWorldState) const
{
  if (nodeType == WorldStateModificationNodeType::PLUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return plusIntOrStr(leftValue, rightValue);
  }
  if (nodeType == WorldStateModificationNodeType::MINUS)
  {
    auto leftValue = leftOperand->getValue(pWorldState);
    auto rightValue = rightOperand->getValue(pWorldState);
    return minusIntOrStr(leftValue, rightValue);
  }
  return "";
}


std::unique_ptr<WorldStateModification> WorldStateModificationNode::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<WorldStateModificationNode>(
        nodeType,
        leftOperand ? leftOperand->clone(pParametersPtr) : std::unique_ptr<WorldStateModification>(),
        rightOperand ? rightOperand->clone(pParametersPtr) : std::unique_ptr<WorldStateModification>(),
        parameterName);
}


std::unique_ptr<WorldStateModification> WorldStateModificationNode::cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const
{
  return std::make_unique<WorldStateModificationNode>(
        nodeType,
        leftOperand ? leftOperand->cloneParamSet(pParameters) : std::unique_ptr<WorldStateModification>(),
        rightOperand ? rightOperand->cloneParamSet(pParameters) : std::unique_ptr<WorldStateModification>(),
        parameterName);
}

std::string WorldStateModificationNode::toStr() const
{
  std::string leftOperandStr;
  if (leftOperand)
    leftOperandStr = leftOperand->toStr();
  std::string rightOperandStr;
  if (rightOperand)
    rightOperandStr = rightOperand->toStr();

  switch (nodeType)
  {
  case WorldStateModificationNodeType::AND:
    return leftOperandStr + " & " + rightOperandStr;
  case WorldStateModificationNodeType::SET:
    return _setFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::FOR_ALL:
    return _forAllFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::ADD:
    return _addFunctionName + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case WorldStateModificationNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  }
  return "";
}



WorldStateModificationFact::WorldStateModificationFact(const FactOptional& pFactOptional)
  : WorldStateModification(WorldStateModificationType::FACT),
    factOptional(pFactOptional)
{
}

bool WorldStateModificationFact::hasFact(const cp::Fact& pFact) const
{
  return factOptional.fact == pFact;
}

bool WorldStateModificationFact::canModifySomethingInTheWorld() const
{
  return factOptional.isFactNegated ||
      !factOptional.fact.isUnreachable();
}

void WorldStateModificationFact::replaceFact(const cp::Fact& pOldFact,
                                             const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
  else
    factOptional.fact.replaceFactInArguments(pOldFact, pNewFact);
}

bool WorldStateModificationFact::forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback, const WorldState&) const
{
  return pFactCallback(factOptional);
}

bool WorldStateModificationFact::operator==(const WorldStateModification& pOther) const
{
  auto* otherFactPtr = pOther.fcFactPtr();
  return otherFactPtr != nullptr &&
      factOptional == otherFactPtr->factOptional;
}

std::string WorldStateModificationFact::getValue(const WorldState& pWorldState) const
{
  return pWorldState.getFactValue(factOptional.fact);
}

std::unique_ptr<WorldStateModification> WorldStateModificationFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<WorldStateModificationFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.replaceArguments(*pParametersPtr);
  return res;
}


std::unique_ptr<WorldStateModification> WorldStateModificationFact::cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const
{
  auto res = std::make_unique<WorldStateModificationFact>(factOptional);
  res->factOptional.fact.replaceArguments(pParameters);
  return res;
}



WorldStateModificationNumber::WorldStateModificationNumber(int pNb)
  : WorldStateModification(WorldStateModificationType::NUMBER),
    nb(pNb)
{
}

bool WorldStateModificationNumber::operator==(const WorldStateModification& pOther) const
{
  auto* otherNumberPtr = pOther.fcNumberPtr();
  return otherNumberPtr != nullptr &&
      nb == otherNumberPtr->nb;
}

std::string WorldStateModificationNumber::getValue(const WorldState& pWorldState) const
{
  return toStr();
}


std::unique_ptr<WorldStateModification> WorldStateModificationNumber::clone(const std::map<std::string, std::string>*) const
{
  return std::make_unique<WorldStateModificationNumber>(nb);
}

std::unique_ptr<WorldStateModification> WorldStateModificationNumber::cloneParamSet(const std::map<std::string, std::set<std::string>>&) const
{
  return std::make_unique<WorldStateModificationNumber>(nb);
}

std::string WorldStateModificationNumber::toStr() const
{
  std::stringstream ss;
  ss << nb;
  return ss.str();
}


} // !cp
