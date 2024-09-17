#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_SETOFINFERENCES_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_SETOFINFERENCES_HPP

#include <map>
#include "../util/api.hpp"
#include <contextualplanner/types/inference.hpp>
#include <contextualplanner/types/facttoconditions.hpp>
#include <contextualplanner/util/alias.hpp>

namespace cp
{

/// Container of a set of inferences.
struct CONTEXTUALPLANNER_API SetOfInferences
{
  /// Construct the set of inferences.
  SetOfInferences() = default;

  SetOfInferences(const Inference& pInference);

  /**
   * @brief Add an inference to check when the facts or the goals change.
   * @param pInferenceId Identifier of the inference to add.
   * @param pInference Inference to add.
   */
  InferenceId addInference(const Inference& pInference,
                           const InferenceId& pInferenceId = "inference");

  /**
   * @brief Remove an inference.
   * @param pInferenceId Identifier of the action to remove.
   *
   * If the inference is not found, this function will have no effect.
   * No exception will be raised.
   */
  void removeInference(const InferenceId& pInferenceId);


  /// Links to point to inference identifiers.
  struct InferenceLinks
  {
    /// Map of fact conditions to inference idntifiers.
    FactToConditions conditionToInferences{};
    /// Map of negated fact conditions to inference idntifiers.
    FactToConditions notConditionToInferences{};

    bool empty() const { return conditionToInferences.empty() && notConditionToInferences.empty(); }
  };

  bool empty() const { return _inferences.empty() && _reachableInferenceLinks.empty(); }
  /// All inferences of the problem.
  const std::map<InferenceId, Inference>& inferences() const { return _inferences; }
  std::map<InferenceId, Inference>& inferences() { return _inferences; }
  /// Reachable inference links.
  const InferenceLinks& reachableInferenceLinks() const { return _reachableInferenceLinks; }


private:
  /// Map of inference indentifers to inference.
  std::map<InferenceId, Inference> _inferences{};
  /// Reachable inference links.
  InferenceLinks _reachableInferenceLinks{};
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_SETOFINFERENCES_HPP
