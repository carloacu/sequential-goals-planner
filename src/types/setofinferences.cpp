#include <contextualplanner/types/setofinferences.hpp>
#include <contextualplanner/util/util.hpp>

namespace cp
{

SetOfInferences::SetOfInferences(const Inference& pInference)
 : _inferences(),
   _reachableInferenceLinks()
{
  addInference(pInference);
}


InferenceId SetOfInferences::addInference(const Inference& pInference,
                                          const InferenceId& pInferenceId)
{
  auto isIdOkForInsertion = [this](const std::string& pId)
  {
    return _inferences.count(pId) == 0;
  };
  auto newId = incrementLastNumberUntilAConditionIsSatisfied(pInferenceId, isIdOkForInsertion);

  _inferences.emplace(newId, pInference);

  if (pInference.condition)
  {
    pInference.condition->forAll(
          [&](const FactOptional& pFactOptional,
              bool pIgnoreFluent)
    {
      if (pFactOptional.isFactNegated)
        _reachableInferenceLinks.notConditionToInferences.add(pFactOptional.fact, newId, pIgnoreFluent);
      else
        _reachableInferenceLinks.conditionToInferences.add(pFactOptional.fact, newId, pIgnoreFluent);
    }
    );
  }
  return newId;
}


void SetOfInferences::removeInference(const InferenceId& pInferenceId)
{
  auto it = _inferences.find(pInferenceId);
  if (it == _inferences.end())
    return;
  auto& inferenceThatWillBeRemoved = it->second;

  if (inferenceThatWillBeRemoved.condition)
  {
    _reachableInferenceLinks.notConditionToInferences.erase(pInferenceId);
    _reachableInferenceLinks.conditionToInferences.erase(pInferenceId);
  }
  _inferences.erase(it);
}


} // !cp
