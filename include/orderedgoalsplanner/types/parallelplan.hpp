#ifndef INCLUDE_ORDEREDGOALSPLANNER_TYPES_PARALLELPLAN_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_TYPES_PARALLELPLAN_HPP

#include <list>
#include "../util/api.hpp"
#include <orderedgoalsplanner/types/actionstodoinparallel.hpp>


namespace ogp
{

struct ORDEREDGOALSPLANNER_API ParallelPan
{
  /// Ordered list of actions set to do in parallel
  std::list<ActionsToDoInParallel> actionsToDoInParallel;

  /// Goals that the plan satisfies
  std::list<Goal> goals;
};


} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_TYPES_PARALLELPLAN_HPP
