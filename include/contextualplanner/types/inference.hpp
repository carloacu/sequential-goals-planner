#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_INFERENCE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_INFERENCE_HPP

#include <functional>
#include <map>
#include <vector>
#include "../util/api.hpp"
#include <contextualplanner/types/goal.hpp>
#include <contextualplanner/types/setoffacts.hpp>


namespace cp
{

/// Specification what is an inference.
struct CONTEXTUALPLANNER_API Inference
{
  /// Construct an inference.
  Inference(const cp::SetOfFacts& pCondition,
            const cp::SetOfFacts& pFactsToModify,
            const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd = {});

  /**
   * Condition to apply the facts and goals modification.
   * The condition is true if the condition is a sub set of a corresponding world state.
   */
  const cp::SetOfFacts condition;
  /// Facts to add or to remove if the condition is true.
  const cp::SetOfFacts factsToModify;
  /// Goals to add if the condition is true.
  const std::map<int, std::vector<cp::Goal>> goalsToAdd;
  /**
   * If the inference is considered as reachable if this 2 conditions are satisfied:
   * * The inference has other stuff to modify than just adding or removing unreachable facts (that will not modify the world state anyway).
   * * The condition can be satisfied. For example if the condition need an unreachable fact to be true the condition will never be satisfied.
   *
   * An inference not reachable, will never be applied but it can be usefull to motivate to do actions toward a goal.
   */
  const bool isReachable;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_INFERENCE_HPP
