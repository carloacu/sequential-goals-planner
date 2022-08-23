#include <contextualplanner/types/expression.hpp>
#include <contextualplanner/util/arithmeticevaluator.hpp>


namespace cp
{
namespace
{
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

}


bool Expression::isValid(const std::map<std::string, std::string>& pVariablesToValue) const
{
  if (elts.size() >= 3)
  {
    auto it = elts.begin();
    auto val1 = _expressionEltToValue(*it, pVariablesToValue);
    ++it;
    if (it->type != ExpressionElementType::OPERATOR ||
        it->value != "=")
      return false;
    ++it;
    auto val2 = _expressionEltToValue(*it, pVariablesToValue);
    ++it;
    while (it != elts.end())
    {
      if (it->type != ExpressionElementType::OPERATOR)
        return false;
      auto op = it->value;
      ++it;
      if (it == elts.end())
        break;
      auto val3 = _expressionEltToValue(*it, pVariablesToValue);
      if (op == "+" || op == "-")
        val2 = evaluteToStr(val2 + op + val3);
      else
        return false;
      ++it;
    }
    if (it != elts.end() || val1 != val2)
      return false;
  }
  return true;
}


bool areExpsValid(const std::list<Expression>& pExps,
                  const std::map<std::string, std::string>& pVariablesToValue)
{
  for (const auto& currExp : pExps)
    if (!currExp.isValid(pVariablesToValue))
      return false;
  return true;
}



} // !cp
