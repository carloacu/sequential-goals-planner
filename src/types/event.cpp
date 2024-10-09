#include <contextualplanner/types/event.hpp>

namespace cp
{


Event::Event(std::unique_ptr<Condition> pCondition,
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


void Event::updateSuccessionCache(const Domain& pDomain,
                                      const SetOfEventsId& pSetOfEventsIdOfThisEvent,
                                      const EventId& pEventIdOfThisEvent)
{
  WorldStateModificationContainerId containerId;
  containerId.setOfEventsIdToExclude.emplace(pSetOfEventsIdOfThisEvent);
  containerId.eventIdToExclude.emplace(pEventIdOfThisEvent);

  auto optionalFactsToIgnore = condition ? condition->getFactToIgnoreInCorrespondingEffect() : std::set<FactOptional>();
  if (factsToModify)
    factsToModify->updateSuccesions(pDomain, containerId, optionalFactsToIgnore);
}

std::string Event::printSuccessionCache() const
{
  std::string res;
  if (factsToModify)
    factsToModify->printSuccesions(res);
  return res;
}


} // !cp
