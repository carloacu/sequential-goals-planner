#include <contextualplanner/types/factmodification.hpp>
#include <contextualplanner/types/problem.hpp>

namespace cp
{
namespace
{
enum class ExpressionOperator
{
  PLUSPLUS,
  PLUS,
  MINUS,
  EQUAL,
  NOT
};

const std::map<std::string, ExpressionOperator> _strToBeginOfTextOperators
{{"++", ExpressionOperator::PLUSPLUS}};
const std::map<char, ExpressionOperator> _charToOperators
{{'=', ExpressionOperator::EQUAL}, {'+', ExpressionOperator::PLUS}, {'+', ExpressionOperator::MINUS}};
const std::string _setFunctionName = "set";
const std::string _forAllFunctionName = "forAll";


std::unique_ptr<FactModification> _merge(std::list<std::unique_ptr<FactModification>>& pFactModifications)
{
  if (pFactModifications.empty())
    return {};
  if (pFactModifications.size() == 1)
    return std::move(pFactModifications.front());

  auto frontElt = std::move(pFactModifications.front());
  pFactModifications.pop_front();
  return std::make_unique<FactModificationNode>(FactModificationNodeType::AND,
                                                std::move(frontElt),
                                                _merge(pFactModifications));
}


std::unique_ptr<FactModification> _factOptToFactModification(const FactOptional& pFactOptional)
{
  if (!pFactOptional.fact.arguments.empty() ||
      (pFactOptional.fact.name[0] != '+' && pFactOptional.fact.name[0] != '$'))
  {
    if (pFactOptional.fact.name == _setFunctionName &&
        pFactOptional.fact.arguments.size() == 2 &&
        pFactOptional.fact.value.empty())
    {
      return std::make_unique<FactModificationNode>(
            FactModificationNodeType::SET,
            std::make_unique<FactModificationFact>(pFactOptional.fact.arguments[0]),
          std::make_unique<FactModificationFact>(pFactOptional.fact.arguments[1]));
    }

    if (pFactOptional.fact.name == _forAllFunctionName &&
        pFactOptional.fact.arguments.size() == 3 &&
        pFactOptional.fact.value.empty())
    {
      auto forAllEffect = _factOptToFactModification(pFactOptional.fact.arguments[2]);
      if (forAllEffect)
      {
        return std::make_unique<FactModificationNode>(
              FactModificationNodeType::FOR_ALL,
              std::make_unique<FactModificationFact>(pFactOptional.fact.arguments[1]),
            std::move(forAllEffect),
            pFactOptional.fact.arguments[0].fact.toStr());
      }
    }

    return std::make_unique<FactModificationFact>(pFactOptional);
  }
  return {};
}

}


FactModification::FactModification(FactModificationType pType)
 : type(pType)
{
}


std::unique_ptr<FactModification> FactModification::fromStr(const std::string& pStr)
{
  std::vector<FactOptional> vect;
  Fact::splitFactOptional(vect, pStr, '&');
  std::list<std::unique_ptr<FactModification>> factModifications;

  for (auto& currOptFact : vect)
  {
    if (currOptFact.fact.name.empty())
      continue;
    std::string currentToken;
    Expression exp;
    auto fillCurrentToken = [&]
    {
      if (!currentToken.empty())
      {
        exp.elts.emplace_back(ExpressionElementType::VALUE, currentToken);
        currentToken.clear();
      }
    };

    auto factModification = _factOptToFactModification(currOptFact);
    if (factModification)
    {
      factModifications.emplace_back(std::move(factModification));
      continue;
    }

    auto currStr = currOptFact.fact.toStr();
    for (std::size_t i = 0; i < currStr.size();)
    {
      bool needToContinue = false;
      if (i == 0)
      {
        for (const auto& currOp : _strToBeginOfTextOperators)
        {
          if (currStr.compare(0, currOp.first.size(), currOp.first) == 0)
          {
            fillCurrentToken();
            exp.elts.emplace_back(ExpressionElementType::OPERATOR, currOp.first);
            i += currOp.first.size();
            needToContinue = true;
            break;
          }
        }
      }
      if (needToContinue)
        continue;
      for (const auto& charToOp : _charToOperators)
      {
        if (charToOp.first == currStr[i])
        {
          fillCurrentToken();
          exp.elts.emplace_back(ExpressionElementType::OPERATOR, std::string(1, charToOp.first));
          ++i;
          needToContinue = true;
          break;
        }
      }
      if (needToContinue)
        continue;
      if (currStr[i] == '$' && currStr[i+1] == '{')
      {
        auto endOfVar = currStr.find('}', i + 2);
        if (endOfVar != std::string::npos)
        {
          fillCurrentToken();
          auto begPos = i + 2;
          exp.elts.emplace_back(ExpressionElementType::FACT, currStr.substr(begPos, endOfVar - begPos));
          i = endOfVar + 1;
          continue;
        }
      }
      currentToken += currStr[i++];
    }
    fillCurrentToken();
    factModifications.emplace_back(std::make_unique<FactModificationExpression>(exp));
  }

  return _merge(factModifications);
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

  return false;
}

bool FactModificationNode::isDynamic() const
{
  if (nodeType == FactModificationNodeType::SET ||
      nodeType == FactModificationNodeType::FOR_ALL)
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
                                              const Problem& pProblem) const
{
  if (leftOperand && rightOperand && !parameterName.empty())
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    if (leftFactPtr != nullptr)
    {
      std::set<cp::Fact> parameterValues;
      pProblem.forAllInstruction(parameterName, leftFactPtr->factOptional.fact, parameterValues);
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
                                  const std::function<void (const Expression&)>& pExpCallback,
                                  const Problem& pProblem) const
{
  if (nodeType == FactModificationNodeType::AND)
  {
    if (leftOperand)
      leftOperand->forAll(pFactCallback, pExpCallback, pProblem);
    if (rightOperand)
      rightOperand->forAll(pFactCallback, pExpCallback, pProblem);
  }
  else if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional.fact;
      factToCheck.value = pProblem.getFactValue(rightFactPtr->factOptional.fact);
      return pFactCallback(FactOptional(factToCheck));
    }
  }
  else if (nodeType == FactModificationNodeType::FOR_ALL)
  {
    _forAllInstruction(
          [&](const FactModification& pFactModification)
    {
      pFactModification.forAll(pFactCallback, pExpCallback, pProblem);
    }, pProblem);
  }
}


bool FactModificationNode::forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback,
                                                   const Problem& pProblem) const
{
  if (nodeType == FactModificationNodeType::AND)
    return (leftOperand && leftOperand->forAllFactsOptUntilTrue(pFactCallback, pProblem)) ||
        (rightOperand && rightOperand->forAllFactsOptUntilTrue(pFactCallback, pProblem));

  if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional;
      factToCheck.fact.value = pProblem.getFactValue(rightFactPtr->factOptional.fact);
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
        res = pFactModification.forAllFactsOptUntilTrue(pFactCallback, pProblem);
    }, pProblem);
    return res;
  }

  return false;
}

void FactModificationNode::forAllFacts(const std::function<void (const FactOptional&)>& pFactCallback,
                                       const Problem& pProblem) const
{
  if (nodeType == FactModificationNodeType::AND)
  {
    if (leftOperand)
      leftOperand->forAllFacts(pFactCallback, pProblem);
    if (rightOperand)
      rightOperand->forAllFacts(pFactCallback, pProblem);
  }
  else if (nodeType == FactModificationNodeType::SET && leftOperand && rightOperand)
  {
    auto* leftFactPtr = leftOperand->fcFactPtr();
    auto* rightFactPtr = rightOperand->fcFactPtr();
    if (leftFactPtr != nullptr && rightFactPtr != nullptr)
    {
      auto factToCheck = leftFactPtr->factOptional.fact;
      factToCheck.value = pProblem.getFactValue(rightFactPtr->factOptional.fact);
      return pFactCallback(factToCheck);
    }
  }
  else if (nodeType == FactModificationNodeType::FOR_ALL)
  {
    _forAllInstruction(
          [&](const FactModification& pFactModification)
    {
      pFactModification.forAllFacts(pFactCallback, pProblem);
    }, pProblem);
  }
}



bool FactModificationNode::forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const
{
  if (nodeType == FactModificationNodeType::AND)
    return (leftOperand && leftOperand->forAllExpUntilTrue(pExpCallback)) ||
        (rightOperand && rightOperand->forAllExpUntilTrue(pExpCallback));
  return false;
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
    factOptional.fact.replaceFactInParameters(pOldFact, pNewFact);
}

bool FactModificationFact::forAllFactsOptUntilTrue(const std::function<bool (const FactOptional&)>& pFactCallback, const Problem&) const
{
  return pFactCallback(factOptional);
}

void FactModificationFact::forAllFacts(const std::function<void (const FactOptional&)>& pFactCallback, const Problem&) const
{
  pFactCallback(factOptional);
}


std::unique_ptr<FactModification> FactModificationFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<FactModificationFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.fillParameters(*pParametersPtr);
  return res;
}


std::unique_ptr<FactModification> FactModificationFact::cloneParamSet(const std::map<std::string, std::set<std::string>>& pParameters) const
{
  auto res = std::make_unique<FactModificationFact>(factOptional);
  res->factOptional.fact.fillParameters(pParameters);
  return res;
}

FactModificationExpression::FactModificationExpression(const Expression& pExpression)
 : FactModification(FactModificationType::EXPRESSION),
   expression(pExpression)
{
}

bool FactModificationExpression::hasFact(const cp::Fact& pFact) const
{
  for (auto& currElt : expression.elts)
    if (currElt.type == ExpressionElementType::FACT && currElt.value == pFact.toStr())
      return true;
  return false;
}

void FactModificationExpression::replaceFact(const cp::Fact& pOldFact,
                                          const Fact& pNewFact)
{
  for (auto& currElt : expression.elts)
    if (currElt.type == ExpressionElementType::FACT && currElt.value == pOldFact.toStr())
      currElt.value = pNewFact.toStr();
}

std::unique_ptr<FactModification> FactModificationExpression::clone(const std::map<std::string, std::string>*) const
{
  return std::make_unique<FactModificationExpression>(expression);
}

std::unique_ptr<FactModification> FactModificationExpression::cloneParamSet(const std::map<std::string, std::set<std::string>>&) const
{
  return std::make_unique<FactModificationExpression>(expression);
}

} // !cp
