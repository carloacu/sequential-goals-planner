#include <orderedgoalsplanner/types/parallelplan.hpp>

namespace ogp
{


std::list<Goal> ParallelPan::extractSatisiedGoals() const
{
   std::list<Goal> res;
   for (auto& currActionsInParallel : actionsToDoInParallel)
   {
     for (auto& currAction : currActionsInParallel.actions)
     {
       if (currAction.fromGoal &&
           (res.empty() || res.back() != *currAction.fromGoal))
         res.push_back(*currAction.fromGoal);
     }
   }
   return res;
}


std::size_t ParallelPan::cost() const
{
  return actionsToDoInParallel.size();
}


} // !ogp
