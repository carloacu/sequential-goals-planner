#include <contextualplanner/contextualplanner.hpp>
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <contextualplanner/arithmeticevaluator.hpp>


namespace cp
{
namespace
{
const std::map<std::string, ExpressionOperator> _strToBeginOfTextOperators
{{"++", ExpressionOperator::PLUSPLUS}};
const std::map<char, ExpressionOperator> _charToOperators
{{'=', ExpressionOperator::EQUAL}, {'+', ExpressionOperator::PLUS}, {'+', ExpressionOperator::MINUS}};


template<typename CONTAINER_TYPE>
static inline std::string _listOfStrToStr(const CONTAINER_TYPE& pStrs,
                                          const std::string& pSeparator = "\n")
{
  std::string res;
  for (const auto& curStr : pStrs)
  {
    if (!res.empty())
      res += pSeparator;
    res += curStr;
  }
  return res;
}

void _splitFacts(
    std::vector<cp::Fact>& pFacts,
    const std::string& pStr,
    char pSeparator)
{
  std::size_t pos = 0u;
  cp::Fact currFact;
  while (pos < pStr.size())
  {
    pos = currFact.fillFactFromStr(pStr, pos, pSeparator) + 1;
    if (!currFact.name.empty())
    {
      pFacts.emplace_back(std::move(currFact));
      currFact = cp::Fact();
    }
  }
  if (!currFact.name.empty())
    pFacts.emplace_back(std::move(currFact));
}

}


bool SetOfFacts::containsFact(const Fact& pFact) const
{
  if (facts.count(pFact) > 0 || notFacts.count(pFact) > 0)
    return true;
  for (auto& currExp : exps)
    for (auto& currElt : currExp.elts)
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pFact.toStr())
        return true;
  return false;
}

void SetOfFacts::rename(const Fact& pOldFact,
                        const Fact& pNewFact)
{
  auto it = facts.find(pOldFact);
  if (it != facts.end())
  {
    facts.erase(it);
    facts.insert(pNewFact);
  }
  auto itNot = notFacts.find(pOldFact);
  if (itNot != notFacts.end())
  {
    notFacts.erase(itNot);
    notFacts.insert(pNewFact);
  }
  for (auto& currExp : exps)
    for (auto& currElt : currExp.elts)
      if (currElt.type == ExpressionElementType::FACT && currElt.value == pOldFact.toStr())
        currElt.value = pNewFact.toStr();
}



std::list<std::pair<std::string, std::string>> SetOfFacts::toFactsStrs() const
{
  std::list<std::pair<std::string, std::string>> res;
  for (const auto& currFact : facts)
    res.emplace_back(currFact.toStr(), currFact.toStr());
  for (const auto& currNotFact : notFacts)
    res.emplace_back(currNotFact.toStr(), "!" + currNotFact.toStr());
  return res;
}


std::list<std::string> SetOfFacts::toStrs() const
{
  std::list<std::string> res;
  for (const auto& currFact : facts)
    res.emplace_back(currFact.toStr());
  for (const auto& currNotFact : notFacts)
    res.emplace_back("!" + currNotFact.toStr());
  for (const auto& currExp : exps)
  {
    std::string ExpStr;
    for (const auto& currElt : currExp.elts)
    {
      if (currElt.type == ExpressionElementType::FACT)
        ExpStr += "${" +  currElt.value + "}";
      else
        ExpStr += currElt.value;
    }
    if (!ExpStr.empty())
      res.emplace_back(ExpStr);
  }
  return res;
}


std::string SetOfFacts::toStr(const std::string& pSeparator) const
{
  return _listOfStrToStr(toStrs(), pSeparator);
}


SetOfFacts SetOfFacts::fromStr(const std::string& pStr,
                               char pSeparator)
{
  std::vector<cp::Fact> vect;
  _splitFacts(vect, pStr, pSeparator);
  SetOfFacts res;

  for (auto& currFact : vect)
  {
    if (currFact.name.empty())
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

    if (!currFact.parameters.empty() ||
        (currFact.name[0] != '+' && currFact.name[0] != '$'))
    {
      if (currFact.name[0] == '!')
      {
        currFact.name = currFact.name.substr(1, currFact.name.size() - 1);
        res.notFacts.insert(currFact);
      }
      else
      {
        res.facts.insert(currFact);
      }
      continue;
    }

    auto currStr = currFact.toStr();
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
    res.exps.emplace_back(std::move(exp));
  }
  return res;
}

std::vector<cp::Fact> factsFromString(const std::string& pStr,
                                      char pSeparator)
{
  std::vector<cp::Fact> res;
  _splitFacts(res, pStr, pSeparator);
  return res;
}

} // !cp
