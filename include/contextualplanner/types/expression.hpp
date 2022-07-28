#ifndef INCLUDE_CONTEXTUALPLANNER_EXPRESSION_HPP
#define INCLUDE_CONTEXTUALPLANNER_EXPRESSION_HPP

#include <string>
#include <list>
#include <map>
#include <assert.h>
#include "../util/api.hpp"


namespace cp
{



#define ADD_HC_EXPRESSIONELEMENTTYPE_TABLE               \
  ADD_HC_EXPRESSIONELEMENTTYPE(OPERATOR, "operator")     \
  ADD_HC_EXPRESSIONELEMENTTYPE(VALUE, "value")           \
  ADD_HC_EXPRESSIONELEMENTTYPE(FACT, "fact")


#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) a,
enum class ExpressionElementType
{
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE



#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {ExpressionElementType::a, b},
static const std::map<ExpressionElementType, std::string> _expressionElementType_toStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE

#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {b, ExpressionElementType::a},
static const std::map<std::string, ExpressionElementType> _expressionElementType_fromStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE
#undef ADD_HC_EXPRESSIONELEMENTTYPE_TABLE


static inline std::string expressionElementType_toStr
(ExpressionElementType pHcActionType)
{
  return _expressionElementType_toStr.find(pHcActionType)->second;
}

static inline ExpressionElementType expressionElementType_fromStr
(const std::string& pStr)
{
  auto it = _expressionElementType_fromStr.find(pStr);
  if (it != _expressionElementType_fromStr.end())
    return it->second;
  assert(false);
  return ExpressionElementType::OPERATOR;
}






enum class ExpressionOperator
{
  PLUSPLUS,
  PLUS,
  MINUS,
  EQUAL,
  NOT
};



struct CONTEXTUALPLANNER_API ExpressionElement
{
  ExpressionElement(ExpressionElementType pType,
                    const std::string& pValue)
    : type(pType),
      value(pValue)
  {
  }
  bool operator==(const ExpressionElement& pOther) const { return type == pOther.type &&
        value == pOther.value; }
  bool operator!=(const ExpressionElement& pOther) const { return !operator==(pOther); }
  ExpressionElementType type;
  std::string value;
};


struct CONTEXTUALPLANNER_API Expression
{
  bool operator==(const Expression& pOther) const { return elts == pOther.elts; }
  bool operator!=(const Expression& pOther) const { return !operator==(pOther); }
  std::list<ExpressionElement> elts;
};


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_EXPRESSION_HPP
