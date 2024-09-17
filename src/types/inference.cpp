#include <contextualplanner/types/inference.hpp>

namespace cp
{


Inference::Inference(std::unique_ptr<Condition> pCondition,
                     std::unique_ptr<WorldStateModification> pFactsToModify,
                     const std::vector<Parameter>& pParameters,
                     const std::map<int, std::vector<cp::Goal>>& pGoalsToAdd)
  : parameters(pParameters),
    condition(pCondition ? std::move(pCondition) : std::unique_ptr<Condition>()),
    factsToModify(pFactsToModify ? std::move(pFactsToModify) : std::unique_ptr<WorldStateModification>()),
    goalsToAdd(pGoalsToAdd)
{
  assert(condition);
  assert(factsToModify || !goalsToAdd.empty());
}


void Inference::updateSuccessionCache(const Domain& pDomain,
                                      const SetOfInferencesId& pSetOfInferencesIdOfThisInference,
                                      const InferenceId& pInferenceIdOfThisInference)
{
  WorldStateModificationContainerId containerId;
  containerId.setOfInferencesIdToExclude.emplace(pSetOfInferencesIdOfThisInference);
  containerId.inferenceIdToExclude.emplace(pInferenceIdOfThisInference);

  auto optionalFactsToIgnore = condition ? condition->getFactToIgnoreInCorrespondingEffect() : std::set<FactOptional>();
  if (factsToModify)
    factsToModify->updateSuccesions(pDomain, containerId, optionalFactsToIgnore);
}

std::string Inference::printSuccessionCache() const
{
  std::string res;
  if (factsToModify)
    factsToModify->printSuccesions(res);
  return res;
}


} // !cp
