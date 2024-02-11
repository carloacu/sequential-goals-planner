#include <contextualplanner/types/worldstatemodification.hpp>
#include <sstream>
#include <contextualplanner/types/worldstate.hpp>
#include "expressionParsed.hpp"
#include <contextualplanner/util/util.hpp>

namespace cp
{
namespace
{
const char* _assignFunctionName = "assign";
const char* _setFunctionName = "set"; // deprecated
const char* _forAllFunctionName = "forall";
const char* _forAllOldFunctionName = "forAll";
const char* _addFunctionName = "add";
const char* _increaseFunctionName = "increase";
const char* _decreaseFunctionName = "decrease";
const char* _andFunctionName = "and";
const char* _whenFunctionName = "when";
const char* _notFunctionName = "not";


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


enum class WorldStateModificationNodeType
{
  AND,
  ASSIGN,
  FOR_ALL,
  INCREASE,
  DECREASE,
  PLUS,
  MINUS
};


struct WorldStateModificationNode : public WorldStateModification
{
  WorldStateModificationNode(WorldStateModificationNodeType pNodeType,
                             std::unique_ptr<WorldStateModification> pLeftOperand,
                             std::unique_ptr<WorldStateModification> pRightOperand,
                             const std::string& pParameterName = "")
    : WorldStateModification(),
      nodeType(pNodeType),
      leftOperand(std::move(pLeftOperand)),
      rightOperand(std::move(pRightOperand)),
      parameterName(pParameterName)
  {
  }

  std::string toStr() const override;

  bool hasFact(const Fact& pFact) const override
  {
    return (leftOperand && leftOperand->hasFact(pFact)) ||
        (rightOperand && rightOperand->hasFact(pFact));
  }

  bool hasFactOptional(const cp::FactOptional& FactOptional) const override
  {
    return (leftOperand && leftOperand->hasFactOptional(FactOptional)) ||
        (rightOperand && rightOperand->hasFactOptional(FactOptional));
  }

  bool isOnlyASetOfFacts() const override
  {
    if (nodeType == WorldStateModificationNodeType::ASSIGN ||
        nodeType == WorldStateModificationNodeType::FOR_ALL ||
        nodeType == WorldStateModificationNodeType::INCREASE ||
        nodeType == WorldStateModificationNodeType::DECREASE ||
        nodeType == WorldStateModificationNodeType::PLUS ||
        nodeType == WorldStateModificationNodeType::MINUS)
      return false;
    return (leftOperand && leftOperand->isOnlyASetOfFacts()) &&
        (rightOperand && rightOperand->isOnlyASetOfFacts());
  }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override
  {
    if (leftOperand)
      leftOperand->replaceFact(pOldFact, pNewFact);
    if (rightOperand)
      rightOperand->replaceFact(pOldFact, pNewFact);
  }

  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const WorldState& pWorldState) const override;
  void iterateOverAllAccessibleFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                                     const WorldState& pWorldState) const;
  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                       const WorldState& pWorldState) const override;
  bool canSatisfyObjective(const std::function<bool (const FactOptional&, std::map<std::string, std::set<std::string>>*)>& pFactCallback,
                           std::map<std::string, std::set<std::string>>& pParameters,
                           const WorldState& pWorldState) const override;
  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState& pWorldState) const override
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


  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersToArgumentPtr) const override
  {
    return std::make_unique<WorldStateModificationNode>(
          nodeType,
          leftOperand ? leftOperand->clone(pParametersToArgumentPtr) : std::unique_ptr<WorldStateModification>(),
          rightOperand ? rightOperand->clone(pParametersToArgumentPtr) : std::unique_ptr<WorldStateModification>(),
          parameterName);
  }


  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParametersToPossibleArgumentPtr) const override
  {
    return std::make_unique<WorldStateModificationNode>(
          nodeType,
          leftOperand ? leftOperand->cloneParamSet(pParametersToPossibleArgumentPtr) : std::unique_ptr<WorldStateModification>(),
          rightOperand ? rightOperand->cloneParamSet(pParametersToPossibleArgumentPtr) : std::unique_ptr<WorldStateModification>(),
          parameterName);
  }

  WorldStateModificationNodeType nodeType;
  std::unique_ptr<WorldStateModification> leftOperand;
  std::unique_ptr<WorldStateModification> rightOperand;
  std::string parameterName;

private:
  void _forAllInstruction(const std::function<void (const WorldStateModification&)>& pCallback,
                          const WorldState& pWorldState) const;
};


struct WorldStateModificationFact : public WorldStateModification
{
  WorldStateModificationFact(const FactOptional& pFactOptional)
    : WorldStateModification(),
      factOptional(pFactOptional)
  {
  }

  std::string toStr() const override { return factOptional.toStr(); }

  bool hasFact(const cp::Fact& pFact) const override
  {
    return factOptional.fact == pFact;
  }

  bool hasFactOptional(const cp::FactOptional& FactOptional) const override
  {
    return factOptional == FactOptional;
  }

  bool isOnlyASetOfFacts() const override { return true; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override
  {
    if (factOptional.fact == pOldFact)
      factOptional.fact = pNewFact;
    else
      factOptional.fact.replaceFactInArguments(pOldFact, pNewFact);
  }

  void forAll(const std::function<void (const FactOptional&)>& pFactCallback,
              const WorldState&) const override { pFactCallback(factOptional); }

  void iterateOverAllAccessibleFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                                     const WorldState&) const override { pFactCallback(factOptional); }


  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback, const WorldState&) const override
  {
    return pFactCallback(factOptional);
  }

  bool canSatisfyObjective(const std::function<bool (const FactOptional&, std::map<std::string, std::set<std::string>>*)>& pFactCallback,
                           std::map<std::string, std::set<std::string>>&,
                           const WorldState&) const override
  {
    return pFactCallback(factOptional, nullptr);
  }

  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState& pWorldState) const override
  {
    return pWorldState.getFactValue(factOptional.fact);
  }

  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>* pParametersToArgumentPtr) const override
  {
    auto res = std::make_unique<WorldStateModificationFact>(factOptional);
    if (pParametersToArgumentPtr != nullptr)
      res->factOptional.fact.replaceArguments(*pParametersToArgumentPtr);
    return res;
  }

  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>& pParametersToPossibleArgumentPtr) const override
  {
    auto res = std::make_unique<WorldStateModificationFact>(factOptional);
    res->factOptional.fact.replaceArguments(pParametersToPossibleArgumentPtr);
    return res;
  }

  FactOptional factOptional;
};



struct WorldStateModificationNumber : public WorldStateModification
{
  WorldStateModificationNumber(int pNb)
    : WorldStateModification(),
      nb(pNb)
  {
  }

  std::string toStr() const override
  {
    std::stringstream ss;
    ss << nb;
    return ss.str();
  }

  bool hasFact(const cp::Fact&) const override { return false; }
  bool hasFactOptional(const cp::FactOptional&) const override { return false; }
  bool isOnlyASetOfFacts() const override { return false; }

  void replaceFact(const cp::Fact& pOldFact,
                   const Fact& pNewFact) override {}
  void forAll(const std::function<void (const FactOptional&)>&,
              const WorldState&) const override {}
  void iterateOverAllAccessibleFacts(const std::function<void (const FactOptional&)>&,
                                     const WorldState&) const override {}
  bool forAllUntilTrue(const std::function<bool (const FactOptional&)>&,
                       const WorldState&) const override { return false; }
  bool canSatisfyObjective(const std::function<bool (const FactOptional&, std::map<std::string, std::set<std::string>>*)>&,
                           std::map<std::string, std::set<std::string>>&,
                           const WorldState&) const override { return false; }
  bool operator==(const WorldStateModification& pOther) const override;

  std::string getValue(const WorldState&) const override
  {
    return toStr();
  }

  std::unique_ptr<WorldStateModification> clone(const std::map<std::string, std::string>*) const override
  {
    return std::make_unique<WorldStateModificationNumber>(nb);
  }

  std::unique_ptr<WorldStateModification> cloneParamSet(const std::map<std::string, std::set<std::string>>&) const override
  {
    return std::make_unique<WorldStateModificationNumber>(nb);
  }

  int nb;
};

const WorldStateModificationNode* _toWmNode(const WorldStateModification& pOther)
{
  return static_cast<const WorldStateModificationNode*>(&pOther);
}

const WorldStateModificationFact* _toWmFact(const WorldStateModification& pOther)
{
  return static_cast<const WorldStateModificationFact*>(&pOther);
}

const WorldStateModificationNumber* _toWmNumber(const WorldStateModification& pOther)
{
  return static_cast<const WorldStateModificationNumber*>(&pOther);
}


std::string WorldStateModificationNode::toStr() const
{
  std::string leftOperandStr;
  if (leftOperand)
    leftOperandStr = leftOperand->toStr();
  std::string rightOperandStr;
  bool isRightOperandAFactWithoutParameter = false;
  if (rightOperand)
  {
    const auto* rightOperandFactPtr = _toWmFact(*rightOperand);
    if (rightOperandFactPtr != nullptr && rightOperandFactPtr->factOptional.fact.arguments.empty() &&
        rightOperandFactPtr->factOptional.fact.value == "")
      isRightOperandAFactWithoutParameter = true;
    rightOperandStr = rightOperand->toStr();
  }

  switch (nodeType)
  {
  case WorldStateModificationNodeType::AND:
    return leftOperandStr + " & " + rightOperandStr;
  case WorldStateModificationNodeType::ASSIGN:
  {
    if (isRightOperandAFactWithoutParameter)
      rightOperandStr += "()"; // To significate it is a fact
    return std::string(_assignFunctionName) + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  }
  case WorldStateModificationNodeType::FOR_ALL:
    return std::string(_forAllFunctionName) + "(" + parameterName + ", " + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::INCREASE:
    return std::string(_increaseFunctionName) + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::DECREASE:
    return std::string(_decreaseFunctionName) + "(" + leftOperandStr + ", " + rightOperandStr + ")";
  case WorldStateModificationNodeType::PLUS:
    return leftOperandStr + " + " + rightOperandStr;
  case WorldStateModificationNodeType::MINUS:
    return leftOperandStr + " - " + rightOperandStr;
  }
  return "";
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
  else if (nodeType == WorldStateModificationNodeType::ASSIGN && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
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
  else if (nodeType == WorldStateModificationNodeType::INCREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }
  else if (nodeType == WorldStateModificationNodeType::DECREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = minusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }
}


void WorldStateModificationNode::iterateOverAllAccessibleFacts(
    const std::function<void (const FactOptional&)>& pFactCallback,
    const WorldState& pWorldState) const
{
  if (nodeType == WorldStateModificationNodeType::AND)
  {
    if (leftOperand)
      leftOperand->iterateOverAllAccessibleFacts(pFactCallback, pWorldState);
    if (rightOperand)
      rightOperand->iterateOverAllAccessibleFacts(pFactCallback, pWorldState);
  }
  else if (nodeType == WorldStateModificationNodeType::ASSIGN && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      if (factToCheck.fact.value == "")
        factToCheck.fact.value = Fact::anyValue;
      return pFactCallback(factToCheck);
    }
  }
  else if (nodeType == WorldStateModificationNodeType::FOR_ALL)
  {
    _forAllInstruction(
          [&](const WorldStateModification& pWsModification)
    {
      pWsModification.iterateOverAllAccessibleFacts(pFactCallback, pWorldState);
    }, pWorldState);
  }
  else if (nodeType == WorldStateModificationNodeType::INCREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }
  else if (nodeType == WorldStateModificationNodeType::DECREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = minusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
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

  if (nodeType == WorldStateModificationNodeType::ASSIGN && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
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

  if (nodeType == WorldStateModificationNodeType::INCREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }

  if (nodeType == WorldStateModificationNodeType::DECREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = minusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck);
    }
  }

  return false;
}


bool WorldStateModificationNode::canSatisfyObjective(const std::function<bool (const FactOptional&, std::map<std::string, std::set<std::string>>*)>& pFactCallback,
                                                     std::map<std::string, std::set<std::string>>& pParameters,
                                                     const WorldState& pWorldState) const
{
  if (nodeType == WorldStateModificationNodeType::AND)
    return (leftOperand && leftOperand->canSatisfyObjective(pFactCallback, pParameters, pWorldState)) ||
        (rightOperand && rightOperand->canSatisfyObjective(pFactCallback, pParameters, pWorldState));

  if (nodeType == WorldStateModificationNodeType::ASSIGN && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = rightOperand->getValue(pWorldState);
      std::map<std::string, std::set<std::string>> localParameterToFind;

      if (factToCheck.fact.value == "")
      {
        factToCheck.fact.value = "??tmpValueFromSet";
        localParameterToFind[factToCheck.fact.value];
      }
      bool res = pFactCallback(factToCheck, &localParameterToFind);
      if (!localParameterToFind.empty() &&
          !localParameterToFind.begin()->second.empty())
      {
        const auto* wSMFPtr = static_cast<const WorldStateModificationFact*>(&*rightOperand);
        if (wSMFPtr != nullptr)
        {
          auto factWithValueToAssign = wSMFPtr->factOptional.fact;
          factWithValueToAssign.replaceArguments(localParameterToFind);
          factWithValueToAssign.value = *localParameterToFind.begin()->second.begin();

          const auto& factNamesToFacts = pWorldState.factNamesToFacts();
          std::map<std::string, std::set<std::string>> newParameters;
          if (factWithValueToAssign.isInOtherFactsMap(factNamesToFacts, true, &newParameters, &pParameters))
          {
            applyNewParams(pParameters, newParameters);
          }
        }
      }
      return res;
    }
  }

  if (nodeType == WorldStateModificationNodeType::FOR_ALL)
  {
    bool res = false;
    _forAllInstruction(
          [&](const WorldStateModification& pWsModification)
    {
      if (!res)
        res = pWsModification.canSatisfyObjective(pFactCallback, pParameters, pWorldState);
    }, pWorldState);
    return res;
  }

  if (nodeType == WorldStateModificationNodeType::INCREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = plusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck, nullptr);
    }
  }

  if (nodeType == WorldStateModificationNodeType::DECREASE && leftOperand && rightOperand)
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = minusIntOrStr(leftOperand->getValue(pWorldState), rightOperand->getValue(pWorldState));
      return pFactCallback(factToCheck, nullptr);
    }
  }

  return false;
}


bool WorldStateModificationNode::operator==(const WorldStateModification& pOther) const
{
  auto* otherNodePtr = _toWmNode(pOther);
  return otherNodePtr != nullptr &&
      nodeType == otherNodePtr->nodeType &&
      _areEqual(leftOperand, otherNodePtr->leftOperand) &&
      _areEqual(rightOperand, otherNodePtr->rightOperand) &&
      parameterName == otherNodePtr->parameterName;
}

void WorldStateModificationNode::_forAllInstruction(const std::function<void (const WorldStateModification &)> &pCallback,
                                                    const WorldState& pWorldState) const
{
  if (leftOperand && rightOperand && !parameterName.empty())
  {
    auto* leftFactPtr = _toWmFact(*leftOperand);
    if (leftFactPtr != nullptr)
    {
      std::set<cp::Fact> parameterValues;
      pWorldState.extractPotentialArgumentsOfAFactParameter(parameterValues, leftFactPtr->factOptional.fact, parameterName);
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

bool WorldStateModificationFact::operator==(const WorldStateModification& pOther) const
{
  auto* otherFactPtr = _toWmFact(pOther);
  return otherFactPtr != nullptr &&
      factOptional == otherFactPtr->factOptional;
}

bool WorldStateModificationNumber::operator==(const WorldStateModification& pOther) const
{
  auto* otherNumberPtr = _toWmNumber(pOther);
  return otherNumberPtr != nullptr && nb == otherNumberPtr->nb;
}



std::unique_ptr<WorldStateModification> _expressionParsedToWsModification(const ExpressionParsed& pExpressionParsed)
{
  std::unique_ptr<WorldStateModification> res;

  if ((pExpressionParsed.name == _assignFunctionName ||
       pExpressionParsed.name == _setFunctionName) && // set is deprecated
      pExpressionParsed.arguments.size() == 2)
  {
    auto leftOperand = _expressionParsedToWsModification(pExpressionParsed.arguments.front());
    const auto& rightOperandExp = *(++pExpressionParsed.arguments.begin());
    auto rightOperand = _expressionParsedToWsModification(rightOperandExp);

    auto* leftFactPtr = static_cast<WorldStateModificationFact*>(&*leftOperand);
    if (leftFactPtr != nullptr && !leftFactPtr->factOptional.isFactNegated)
    {
      const auto* rightFactPtr = static_cast<const WorldStateModificationFact*>(&*rightOperand);
      if (rightFactPtr != nullptr &&
          rightFactPtr->factOptional.fact.arguments.empty() &&
          rightFactPtr->factOptional.fact.value == "")
      {
        if (rightFactPtr->factOptional.fact.name == Fact::undefinedValue)
        {
          leftFactPtr->factOptional.isFactNegated = true;
          leftFactPtr->factOptional.fact.value = Fact::anyValue;
          res = std::make_unique<WorldStateModificationFact>(std::move(*leftFactPtr));
        }
        else if (pExpressionParsed.name == _assignFunctionName && !rightOperandExp.isAFunction)
        {
          leftFactPtr->factOptional.fact.value = rightFactPtr->factOptional.fact.name;
          res = std::make_unique<WorldStateModificationFact>(std::move(*leftFactPtr));
        }
      }
    }

    if (!res)
      res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::ASSIGN,
                                                         std::move(leftOperand), std::move(rightOperand));
  }
  else if (pExpressionParsed.name == _notFunctionName &&
           pExpressionParsed.arguments.size() == 1)
  {
    auto factNegationed = pExpressionParsed.arguments.front().toFact();
    factNegationed.isFactNegated = !factNegationed.isFactNegated;
    res = std::make_unique<WorldStateModificationFact>(factNegationed);
  }
  else if ((pExpressionParsed.name == _forAllFunctionName || pExpressionParsed.name == _forAllOldFunctionName) &&
           (pExpressionParsed.arguments.size() == 2 || pExpressionParsed.arguments.size() == 3))
  {
    auto itArg = pExpressionParsed.arguments.begin();
    auto& firstArg = *itArg;
    ++itArg;
    auto& secondArg = *itArg;
    if (pExpressionParsed.arguments.size() == 3)
    {
      ++itArg;
      auto& thridArg = *itArg;
      res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::FOR_ALL,
                                                         std::make_unique<WorldStateModificationFact>(secondArg.toFact()),
                                                         _expressionParsedToWsModification(thridArg),
                                                         firstArg.name);
    }
    else if (secondArg.name == _whenFunctionName &&
             secondArg.arguments.size() == 2)
    {
      auto itWhenArg = secondArg.arguments.begin();
      auto& firstWhenArg = *itWhenArg;
      ++itWhenArg;
      auto& secondWhenArg = *itWhenArg;
      res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::FOR_ALL,
                                                         std::make_unique<WorldStateModificationFact>(firstWhenArg.toFact()),
                                                         _expressionParsedToWsModification(secondWhenArg),
                                                         firstArg.name);
    }
  }
  else if (pExpressionParsed.name == _andFunctionName &&
           pExpressionParsed.arguments.size() >= 2)
  {
    std::list<std::unique_ptr<WorldStateModification>> elts;
    for (auto& currExp : pExpressionParsed.arguments)
      elts.emplace_back(_expressionParsedToWsModification(currExp));

    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::AND, std::move(*(--(--elts.end()))), std::move(elts.back()));
    elts.pop_back();
    elts.pop_back();

    while (!elts.empty())
    {
      res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::AND, std::move(elts.back()), std::move(res));
      elts.pop_back();
    }
  }
  else if ((pExpressionParsed.name == _increaseFunctionName || pExpressionParsed.name == _addFunctionName) &&
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

    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::INCREASE,
                                                       _expressionParsedToWsModification(firstArg),
                                                       std::move(rightOpPtr));
  }
  else if (pExpressionParsed.name == _decreaseFunctionName &&
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

    res = std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::DECREASE,
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


std::unique_ptr<WorldStateModification> WorldStateModification::fromStr(const std::string& pStr)
{
  if (pStr.empty())
    return {};
  std::size_t pos = 0;
  auto expressionParsed = ExpressionParsed::fromStr(pStr, pos);
  return _expressionParsedToWsModification(expressionParsed);
}


std::unique_ptr<WorldStateModification> WorldStateModification::createByConcatenation(const WorldStateModification& pWsModif1,
                                                                                      const WorldStateModification& pWsModif2)
{
  return std::make_unique<WorldStateModificationNode>(WorldStateModificationNodeType::AND,
                                                      pWsModif1.clone(nullptr),
                                                      pWsModif2.clone(nullptr));
}


} // !cp
