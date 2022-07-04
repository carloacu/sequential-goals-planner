#ifndef INCLUDE_TRACKERS_FACTSCHANGEDTRACKER_HPP
#define INCLUDE_TRACKERS_FACTSCHANGEDTRACKER_HPP

#include <set>
#include <string>
#include "../api.hpp"
#include <contextualplanner/observableunsafe.hpp>
#include <contextualplanner/problem.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API FactsChangedTracker
{
  FactsChangedTracker(const Problem& pProblem);
  ~FactsChangedTracker();
  cpstd::observable::ObservableUnsafe<void (const std::set<cp::Fact>&)> onFactsAdded{};
  cpstd::observable::ObservableUnsafe<void (const std::set<cp::Fact>&)> onFactsRemoved{};


private:
  std::set<cp::Fact> _existingFacts;
  cpstd::observable::Connection _onFactsChangedConneection;
};

} // !cp


#endif // INCLUDE_TRACKERS_FACTSCHANGEDTRACKER_HPP
