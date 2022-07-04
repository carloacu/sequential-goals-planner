#include <contextualplanner/trackers/factsaddedtracker.hpp>
#include <contextualplanner/fact.hpp>

namespace cp
{


FactsAddedTracker::FactsAddedTracker(const Problem& pProblem)
  : _existingFacts(),
    _onFactsChangedConneection(
      pProblem.onFactsChanged.connectUnsafe([this](const std::set<Fact>& pFacts) {
  std::set<Fact> newFacts;
  for (const auto& currFact : pFacts)
  {
    if (_existingFacts.count(currFact) == 0)
      newFacts.insert(currFact);
  }

  if (!newFacts.empty())
    onFactsAdded(newFacts);
  _existingFacts = pFacts;
}))
{
}


FactsAddedTracker::~FactsAddedTracker()
{
  _onFactsChangedConneection.disconnect();
}



} // !cp

