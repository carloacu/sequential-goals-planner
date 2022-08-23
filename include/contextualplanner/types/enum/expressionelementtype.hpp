#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ENUM_EXPRESSIONELEMENTTYPE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ENUM_EXPRESSIONELEMENTTYPE_HPP

#include <string>
#include <map>
#include <assert.h>
#include "../../util/api.hpp"


namespace cp
{

// This macro is for mapping enum values with their string value.
#define ADD_HC_EXPRESSIONELEMENTTYPE_TABLE               \
  ADD_HC_EXPRESSIONELEMENTTYPE(OPERATOR, "operator")     \
  ADD_HC_EXPRESSIONELEMENTTYPE(VALUE, "value")           \
  ADD_HC_EXPRESSIONELEMENTTYPE(FACT, "fact")


/// Type of a part of an Expression.
#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) a,
enum class ExpressionElementType
{
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE


// Private map to convert enum values to string.
#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {ExpressionElementType::a, b},
static const std::map<ExpressionElementType, std::string> _expressionElementType_toStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE

// Private map to convert strings to enum value.
#define ADD_HC_EXPRESSIONELEMENTTYPE(a, b) {b, ExpressionElementType::a},
static const std::map<std::string, ExpressionElementType> _expressionElementType_fromStr = {
  ADD_HC_EXPRESSIONELEMENTTYPE_TABLE
};
#undef ADD_HC_EXPRESSIONELEMENTTYPE
#undef ADD_HC_EXPRESSIONELEMENTTYPE_TABLE


/**
 * @brief Get a string from a ExpressionElementType enum value.
 * @param pExpressionElementType ExpressionElementType enum value.
 * @return String corresponding to the enum value.
 */
static inline std::string expressionElementType_toStr
(ExpressionElementType pExpressionElementType)
{
  return _expressionElementType_toStr.find(pExpressionElementType)->second;
}



/**
 * @brief Get a ExpressionElementType enum value from a string.
 * @param pStr String corresponding to an ExpressionElementType enum value.
 * @return ExpressionElementType enum value.
 * Note: If the string does not match, in debug an assertion is raised, and in release the value ExpressionElementType::OPERATOR is returned.
 */
static inline ExpressionElementType expressionElementType_fromStr
(const std::string& pStr)
{
  auto it = _expressionElementType_fromStr.find(pStr);
  if (it != _expressionElementType_fromStr.end())
    return it->second;
  assert(false);
  return ExpressionElementType::OPERATOR;
}


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ENUM_EXPRESSIONELEMENTTYPE_HPP
