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

    if (!currOptFact.fact.parameters.empty() ||
        (currOptFact.fact.name[0] != '+' && currOptFact.fact.name[0] != '$'))
    {
      if (currOptFact.fact.name == "forAll" &&
          currOptFact.fact.parameters.size() == 2 &&
          currOptFact.fact.value.empty())
      {
        factModifications.emplace_back(std::make_unique<FactModificationNode>(
                                      FactModificationNodeType::FOR_ALL,
                                      std::make_unique<FactModificationFact>(currOptFact.fact.parameters[0]),
                                    std::make_unique<FactModificationFact>(currOptFact.fact.parameters[1])));
      }
      else
      {
        factModifications.emplace_back(std::make_unique<FactModificationFact>(currOptFact));
      }
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
                                           std::unique_ptr<FactModification> pRightOperand)
  : FactModification(FactModificationType::NODE),
   nodeType(pNodeType),
   leftOperand(std::move(pLeftOperand)),
   rightOperand(std::move(pRightOperand))
{
}

bool FactModificationNode::hasFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->hasFact(pFact)) ||
      (rightOperand && rightOperand->hasFact(pFact));
}

bool FactModificationNode::canModifySomethingInTheWorld() const
{
  return (leftOperand && leftOperand->canModifySomethingInTheWorld()) ||
      (rightOperand && rightOperand->canModifySomethingInTheWorld());
}

bool FactModificationNode::containsFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->containsFact(pFact)) ||
      (rightOperand && rightOperand->containsFact(pFact));
}

bool FactModificationNode::containsNotFact(const Fact& pFact) const
{
  return (leftOperand && leftOperand->containsNotFact(pFact)) ||
      (rightOperand && rightOperand->containsNotFact(pFact));
}

bool FactModificationNode::containsExpression(const Expression& pExpression) const
{
  return (leftOperand && leftOperand->containsExpression(pExpression)) ||
      (rightOperand && rightOperand->containsExpression(pExpression));
}


void FactModificationNode::replaceFact(const cp::Fact& pOldFact,
                                    const Fact& pNewFact)
{
  if (leftOperand)
    leftOperand->replaceFact(pOldFact, pNewFact);
  if (rightOperand)
    rightOperand->replaceFact(pOldFact, pNewFact);
}


void FactModificationNode::forAll(const std::function<void (const FactOptional&)>& pFactCallback,
                                  const std::function<void (const Expression&)>& pExpCallback) const
{
  if (leftOperand)
    leftOperand->forAll(pFactCallback, pExpCallback);
  if (rightOperand)
    rightOperand->forAll(pFactCallback, pExpCallback);
}

void FactModificationNode::forAllFacts(const std::function<void (const Fact&)>& pFactCallback) const
{
  if (leftOperand)
    leftOperand->forAllFacts(pFactCallback);
  if (rightOperand)
    rightOperand->forAllFacts(pFactCallback);
}

bool FactModificationNode::forAllFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback) const
{
  return (leftOperand && leftOperand->forAllFactsUntilTrue(pFactCallback)) ||
      (rightOperand && rightOperand->forAllFactsUntilTrue(pFactCallback));
}

void FactModificationNode::forAllNotFacts(const std::function<void (const Fact&)>& pFactCallback) const
{
  if (leftOperand)
    leftOperand->forAllNotFacts(pFactCallback);
  if (rightOperand)
    rightOperand->forAllNotFacts(pFactCallback);
}

bool FactModificationNode::forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback) const
{
  return (leftOperand && leftOperand->forAllNotFactsUntilTrue(pFactCallback)) ||
      (rightOperand && rightOperand->forAllNotFactsUntilTrue(pFactCallback));
}

bool FactModificationNode::forAllExpUntilTrue(const std::function<bool (const Expression&)>& pExpCallback) const
{
  return (leftOperand && leftOperand->forAllExpUntilTrue(pExpCallback)) ||
      (rightOperand && rightOperand->forAllExpUntilTrue(pExpCallback));
}


bool FactModificationNode::untilFalse(const std::function<bool (const FactOptional&)>& pFactCallback,
                                      const std::function<bool (const Expression&)>& pExpCallback,
                                      const Problem& pProblem) const
{
  return true;
}



std::unique_ptr<FactModification> FactModificationNode::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<FactModificationNode>(
        nodeType,
        leftOperand ? leftOperand->clone(pParametersPtr) : std::unique_ptr<FactModification>(),
        rightOperand ? rightOperand->clone(pParametersPtr) : std::unique_ptr<FactModification>());
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

bool FactModificationFact::containsFact(const Fact& pFact) const
{
  return !factOptional.isFactNegated && factOptional.fact == pFact;
}

bool FactModificationFact::containsNotFact(const Fact& pFact) const
{
  return factOptional.isFactNegated && factOptional.fact == pFact;
}

void FactModificationFact::replaceFact(const cp::Fact& pOldFact,
                                    const Fact& pNewFact)
{
  if (factOptional.fact == pOldFact)
    factOptional.fact = pNewFact;
}

void FactModificationFact::forAllFacts(const std::function<void (const Fact&)>& pFactCallback) const
{
  if (!factOptional.isFactNegated)
    pFactCallback(factOptional.fact);
}

bool FactModificationFact::forAllFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback) const
{
  if (!factOptional.isFactNegated)
    return pFactCallback(factOptional.fact);
  return false;
}

void FactModificationFact::forAllNotFacts(const std::function<void (const Fact&)>& pFactCallback) const
{
  if (factOptional.isFactNegated)
    pFactCallback(factOptional.fact);
}

bool FactModificationFact::forAllNotFactsUntilTrue(const std::function<bool (const Fact&)>& pFactCallback) const
{
  if (factOptional.isFactNegated)
    return pFactCallback(factOptional.fact);
  return false;
}


std::unique_ptr<FactModification> FactModificationFact::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  auto res = std::make_unique<FactModificationFact>(factOptional);
  if (pParametersPtr != nullptr)
    res->factOptional.fact.fillParameters(*pParametersPtr);
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

bool FactModificationExpression::containsExpression(const Expression& pExpression) const
{
  return expression == pExpression;
}

void FactModificationExpression::replaceFact(const cp::Fact& pOldFact,
                                          const Fact& pNewFact)
{
  for (auto& currElt : expression.elts)
    if (currElt.type == ExpressionElementType::FACT && currElt.value == pOldFact.toStr())
      currElt.value = pNewFact.toStr();
}

std::unique_ptr<FactModification> FactModificationExpression::clone(const std::map<std::string, std::string>* pParametersPtr) const
{
  return std::make_unique<FactModificationExpression>(expression);
}


} // !cp
