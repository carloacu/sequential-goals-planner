#ifndef PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP
#define PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP

#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <prioritizedgoalsplanner/util/alias.hpp>

namespace pgp
{
struct ActionInvocationWithGoal;
struct Domain;
struct Goal;
struct Problem;
struct WorldState;


std::list<std::list<pgp::ActionInvocationWithGoal>> toParallelPlan
(std::list<pgp::ActionInvocationWithGoal>& pSequentialPlan,
 bool pParalleliseOnyFirstStep,
 const pgp::Problem& pProblem,
 const pgp::Domain& pDomain,
 const std::list<pgp::Goal>& pGoals,
 const std::unique_ptr<std::chrono::_V2::steady_clock::time_point>& pNow);

} // End of namespace pgp


#endif // PRIORITIZEDGOALSPLANNER_SRC_ALGO_CONVERTTOPARALLELPLAN_HPP
