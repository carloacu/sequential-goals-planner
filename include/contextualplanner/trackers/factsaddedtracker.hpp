#ifndef INCLUDE_TRACKERS_FACTSADDEDTRACKER_HPP
#define INCLUDE_TRACKERS_FACTSADDEDTRACKER_HPP

#include <set>
#include <string>
#include "../api.hpp"
#include <contextualplanner/observableunsafe.hpp>
#include <contextualplanner/problem.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API FactsAddedTracker
{
  FactsAddedTracker(const Problem& pProblem);
  ~FactsAddedTracker();
  cpstd::observable::ObservableUnsafe<void (const std::set<cp::Fact>&)> onFactsAdded{};


private:
  std::set<cp::Fact> _existingFacts;
  cpstd::observable::Connection _onFactsChangedConneection;
};

} // !cp


#endif // INCLUDE_TRACKERS_FACTSADDEDTRACKER_HPP
