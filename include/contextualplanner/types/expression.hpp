#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_EXPRESSION_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_EXPRESSION_HPP

#include <string>
#include <list>
#include <map>
#include <assert.h>
#include "../util/api.hpp"
#include <contextualplanner/types/enum/expressionelementtype.hpp>

namespace cp
{

/// A part of an Expression.
struct CONTEXTUALPLANNER_API ExpressionElement
{
  /**
   * @brief Check equality with another expression element.
   * @param pType Expression element type.
   * @param pValue Value of the expression element.
   */
  ExpressionElement(ExpressionElementType pType,
                    const std::string& pValue)
    : type(pType),
      value(pValue)
  {
  }

  /// Check equality with another ExpressionElement.
  bool operator==(const ExpressionElement& pOther) const { return type == pOther.type &&
        value == pOther.value; }
  /// Check not equality with another ExpressionElement.
  bool operator!=(const ExpressionElement& pOther) const { return !operator==(pOther); }

  /// Expression element type.
  ExpressionElementType type;
  /// Value of the expression element.
  std::string value;
};


/// Expression for making arithmetic comparisons between facts
struct CONTEXTUALPLANNER_API Expression
{
  /// Check equality with another Expression.
  bool operator==(const Expression& pOther) const { return elts == pOther.elts; }
  /// Check not equality with another Expression.
  bool operator!=(const Expression& pOther) const { return !operator==(pOther); }

  /// Elements of the expression.
  std::list<ExpressionElement> elts;

  /**
   * @brief Is the expression valid according to a context.
   * @param pVariablesToValue Variables to value.
   * @return True if the expression is valid according to the map of variables to value.
   */
  bool isValid(const std::map<std::string, std::string>& pVariablesToValue) const;
};


/**
 * @brief Are the expressions valid according to a context.
 * @param pExps The expressions.
 * @param pVariablesToValue Variables to value.
 * @return True if the expression is valid according to the map of variables to value.
 */
bool areExpsValid(const std::list<Expression>& pExps,
                  const std::map<std::string, std::string>& pVariablesToValue);


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_EXPRESSION_HPP
