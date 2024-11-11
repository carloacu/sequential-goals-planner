#ifndef PRIORITIZEDGOALSPLANNER_SRC_ALGO_NOTIFYACTIONDONE_HPP
#define PRIORITIZEDGOALSPLANNER_SRC_ALGO_NOTIFYACTIONDONE_HPP


#include <chrono>
#include <map>
#include <memory>
#include <vector>
#include <prioritizedgoalsplanner/util/alias.hpp>

namespace pgp
{
struct Action;
struct ActionInvocationWithGoal;
struct Domain;
struct Goal;
struct Problem;
struct Historical;
struct LookForAnActionOutputInfos;
struct SetOfEvents;
struct WorldStateModification;
struct Ontology;
struct SetOfEntities;


void notifyActionInvocationDone(Problem& pProblem,
                                bool& pGoalChanged,
                                const std::map<SetOfEventsId, SetOfEvents>& pSetOfEvents,
                                const ActionInvocationWithGoal& pOnStepOfPlannerResult,
                                const std::unique_ptr<WorldStateModification>& pEffect,
                                const Ontology& pOntology,
                                const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                                const std::map<int, std::vector<Goal>>* pGoalsToAdd,
                                const std::vector<Goal>* pGoalsToAddInCurrentPriority,
                                LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr);


void updateProblemForNextPotentialPlannerResult(
    Problem& pProblem,
    bool& pGoalChanged,
    const ActionInvocationWithGoal& pOneStepOfPlannerResult,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr);


void updateProblemForNextPotentialPlannerResultWithAction(
    Problem& pProblem,
    bool& pGoalChanged,
    const ActionInvocationWithGoal& pOneStepOfPlannerResult,
    const Action& pOneStepAction,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical,
    LookForAnActionOutputInfos* pLookForAnActionOutputInfosPtr);


} // End of namespace pgp


#endif // PRIORITIZEDGOALSPLANNER_SRC_ALGO_NOTIFYACTIONDONE_HPP
