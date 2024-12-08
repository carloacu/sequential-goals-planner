#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_SETOFCALLBACKS_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_SETOFCALLBACKS_HPP

#include <map>
#include "../util/api.hpp"
#include <orderedgoalsplanner/types/conditiontocallback.hpp>
#include <orderedgoalsplanner/types/factstovalue.hpp>
#include <orderedgoalsplanner/util/alias.hpp>

namespace ogp
{

/// Container of a set of conditions to callback.
struct ORDEREDGOALSPLANNER_API SetOfCallbacks
{
  SetOfCallbacks() = default;

  CallbackId add(const ConditionToCallback& pConditionToCallback,
                 const CallbackId& pCallbackId = "callback");

  void remove(const CallbackId& pCallbackId);


  struct CallbackLinks
  {
    /// Map of fact conditions to ConditionToCallback identifiers.
    FactsToValue conditionToCallbacks{};
    /// Map of negated fact conditions to ConditionToCallback identifiers.
    FactsToValue notConditionToCallbacks{};

    bool empty() const { return conditionToCallbacks.empty() && notConditionToCallbacks.empty(); }
  };

  bool empty() const { return _callbacks.empty() && _reachableCallbackLinks.empty(); }
  const std::map<CallbackId, ConditionToCallback>& callbacks() const { return _callbacks; }
  std::map<CallbackId, ConditionToCallback>& callbacks() { return _callbacks; }
  const CallbackLinks& reachableCallbackLinks() const { return _reachableCallbackLinks; }


private:
  std::map<CallbackId, ConditionToCallback> _callbacks{};
  CallbackLinks _reachableCallbackLinks{};
};

} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_SETOFCALLBACKS_HPP
