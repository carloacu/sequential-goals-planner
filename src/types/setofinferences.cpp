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
  for (const auto& currFact : pInference.condition.facts)
    links.conditionToInferences[currFact.name].insert(pInferenceId);
  for (const auto& currNotFact : pInference.condition.notFacts)
    links.notConditionToInferences[currNotFact.name].insert(pInferenceId);
}


void SetOfInferences::removeInference(const InferenceId& pInferenceId)
{
  auto it = _inferences.find(pInferenceId);
  if (it == _inferences.end())
    return;
  auto& inferenceThatWillBeRemoved = it->second;
  auto& links = inferenceThatWillBeRemoved.isReachable ? _reachableInferenceLinks : _unreachableInferenceLinks;
  for (const auto& currFact : inferenceThatWillBeRemoved.condition.facts)
    links.conditionToInferences[currFact.name].erase(pInferenceId);
  for (const auto& currFact : inferenceThatWillBeRemoved.condition.notFacts)
    links.notConditionToInferences[currFact.name].erase(pInferenceId);
  _inferences.erase(it);
}


} // !cp
