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
  cp::SetOfFacts condition;
  /// Punctual facts to have to the condition validated.
  std::set<Fact> punctualFactsCondition;
  /// Facts to add or to remove if the condition is true.
  cp::SetOfFacts factsToModify;
  /// Goals to add if the condition is true.
  std::map<int, std::vector<cp::Goal>> goalsToAdd;
};



} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_INFERENCE_HPP
