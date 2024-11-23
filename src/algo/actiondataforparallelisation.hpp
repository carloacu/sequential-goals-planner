#ifndef ORDEREDGOALSPLANNER_SRC_ALGO_ACTIONDATAFORPARALLELISATION_HPP
#define ORDEREDGOALSPLANNER_SRC_ALGO_ACTIONDATAFORPARALLELISATION_HPP

#include <list>
#include <memory>
#include <set>
#include <orderedgoalsplanner/types/actioninvocationwithgoal.hpp>

namespace ogp
{
struct Action;
struct Condition;
struct FactOptional;
struct Problem;
struct WorldStateModification;


struct ActionDataForParallelisation
{
  ActionDataForParallelisation(const Action& pAction, ActionInvocationWithGoal&& pActionInvWithGoal);

  const Condition* getConditionWithoutParameterPtr();

  const WorldStateModification* getWorldStateModificationAtStartWithoutParameterPtr();

  const WorldStateModification* getWorldStateModificationWithoutParameterPtr();

  const WorldStateModification* getPotentialWorldStateModificationWithoutParameterPtr();

  const std::set<FactOptional>& getAllOptFactsThatCanBeModified();

  bool hasAContradictionWithAnEffect(const std::set<FactOptional>& pFactsOpt);

  bool canBeInParallel(ActionDataForParallelisation& pOther);

  bool canBeInParallelOfList(std::list<ActionDataForParallelisation>& pOthers);

  const Action& action;
  ActionInvocationWithGoal actionInvWithGoal;
  std::unique_ptr<Condition> conditionWithParameterFilled;
  std::unique_ptr<WorldStateModification> worldStateModificationAtStartWithParameterFilled;
  std::unique_ptr<WorldStateModification> worldStateModificationWithParameterFilled;
  std::unique_ptr<WorldStateModification> potentialWorldStateModificationWithParameterFilled;
  std::unique_ptr<std::set<FactOptional>> factsThatCanBeModifiedPtr;
};


std::list<Goal> extractSatisfiedGoals(Problem& pProblem,
                                      const Domain& pDomain,
                                      std::list<std::list<ActionDataForParallelisation>>::iterator pCurrItInPlan,
                                      std::list<std::list<ActionDataForParallelisation>>& pPlan,
                                      const ActionDataForParallelisation* pActionToSkipPtr,
                                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


} // End of namespace ogp


#endif // ORDEREDGOALSPLANNER_SRC_ALGO_ACTIONDATAFORPARALLELISATION_HPP
