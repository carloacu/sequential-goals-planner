#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_CONDITIONTOCALLBACK_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_CONDITIONTOCALLBACK_HPP

#include <assert.h>
#include <functional>
#include <vector>
#include "../util/api.hpp"
#include <orderedgoalsplanner/types/condition.hpp>

namespace ogp
{

/// Have a callback called when a condition becomes true.
struct ORDEREDGOALSPLANNER_API ConditionToCallback
{
  ConditionToCallback(std::unique_ptr<Condition> pCondition,
                      const std::function<void()>& pCallback,
                      const std::vector<Parameter>& pParameters = {})
    : parameters(pParameters),
      condition(pCondition ? pCondition->clone() : std::unique_ptr<Condition>()),
      callback(pCallback)
  {
    assert(condition);
  }

  ConditionToCallback(const ConditionToCallback& pOther)
    : parameters(pOther.parameters),
      condition(pOther.condition ? pOther.condition->clone() : std::unique_ptr<Condition>()),
      callback(pOther.callback)
  {
    assert(condition);
  }

  /// Parameter names.
  std::vector<Parameter> parameters;
  /// Condition to call the callback.
  const std::unique_ptr<Condition> condition;
  /// Callback.
  std::function<void()> callback;
};



} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_CONDITIONTOCALLBACK_HPP
