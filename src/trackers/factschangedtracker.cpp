#include <contextualplanner/trackers/factschangedtracker.hpp>
#include <contextualplanner/fact.hpp>

namespace cp
{


FactsChangedTracker::FactsChangedTracker(const Problem& pProblem)
  : onFactsAdded(),
    onFactsRemoved(),
    _existingFacts(),
    _onFactsChangedConneection(
      pProblem.onFactsChanged.connectUnsafe([this](const std::set<Fact>& pFacts) {
  std::set<Fact> newFacts;
  for (const auto& currFact : pFacts)
  {
    auto it = _existingFacts.find(currFact);
    if (it != _existingFacts.end())
      _existingFacts.erase(it);
    else
      newFacts.insert(currFact);
  }

  if (!_existingFacts.empty())
    onFactsRemoved(_existingFacts);
  if (!newFacts.empty())
    onFactsAdded(newFacts);
  _existingFacts = pFacts;
}))
{
}


FactsChangedTracker::~FactsChangedTracker()
{
  _onFactsChangedConneection.disconnect();
}



} // !cp

