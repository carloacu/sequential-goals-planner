#include <contextualplanner/types/setofinferences.hpp>

namespace cp
{


void SetOfInferences::addInference(const InferenceId& pInferenceId,
                                   const Inference& pInference)
{
  if (_inferences.count(pInferenceId) > 0)
    return;
  _inferences.emplace(pInferenceId, pInference);
  auto& links = pInference.isReachable ? _reachableInferenceLinks : _unreachableInferenceLinks;

  if (pInference.condition)
  {
    pInference.condition->forAll(
          [&](const FactOptional& pFactOptional)
    {
      if (pFactOptional.isFactNegated)
        links.notConditionToInferences[pFactOptional.fact.name].insert(pInferenceId);
      else
        links.conditionToInferences[pFactOptional.fact.name].insert(pInferenceId);
    }
    );
  }
}


void SetOfInferences::removeInference(const InferenceId& pInferenceId)
{
  auto it = _inferences.find(pInferenceId);
  if (it == _inferences.end())
    return;
  auto& inferenceThatWillBeRemoved = it->second;
  auto& links = inferenceThatWillBeRemoved.isReachable ? _reachableInferenceLinks : _unreachableInferenceLinks;

  if (inferenceThatWillBeRemoved.condition)
  {
    inferenceThatWillBeRemoved.condition->forAll(
          [&](const FactOptional& pFactOptional)
    {
      if (pFactOptional.isFactNegated)
        links.notConditionToInferences[pFactOptional.fact.name].erase(pInferenceId);
      else
        links.conditionToInferences[pFactOptional.fact.name].erase(pInferenceId);
    }
    );
  }
  _inferences.erase(it);
}


} // !cp
