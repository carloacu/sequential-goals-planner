#ifndef PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP
#define PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP

#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <prioritizedgoalsplanner/util/alias.hpp>

namespace cp
{
struct ActionInvocationWithGoal;
struct Domain;
struct Goal;
struct Problem;
struct WorldState;


std::list<std::list<cp::ActionInvocationWithGoal>> toParallelPlan
(std::list<cp::ActionInvocationWithGoal>& pSequentialPlan,
 bool pParalleliseOnyFirstStep,
 const cp::Problem& pProblem,
 const cp::Domain& pDomain,
 const std::list<cp::Goal>& pGoals,
 const std::unique_ptr<std::chrono::_V2::steady_clock::time_point>& pNow);

} // End of namespace cp


#endif // PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP
