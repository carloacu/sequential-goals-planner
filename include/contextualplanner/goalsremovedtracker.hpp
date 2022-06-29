#ifndef INCLUDE_GOALSREMOVeDTRACKER_HPP
#define INCLUDE_GOALSREMOVeDTRACKER_HPP

#include <set>
#include <string>
#include "api.hpp"
#include <contextualplanner/observableunsafe.hpp>
#include <contextualplanner/problem.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API GoalsRemovedTracker
{
  GoalsRemovedTracker(const Problem& pProblem);
  ~GoalsRemovedTracker();
  cpstd::observable::ObservableUnsafe<void (const std::set<std::string>&)> onGoalsRemoved{};


private:
  std::set<std::string> _existingGoals;
  cpstd::observable::Connection _onGoalsChangedConneection;
};

} // !cp


#endif // INCLUDE_GOALSREMOVeDTRACKER_HPP
