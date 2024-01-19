#ifndef INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
#define INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP

#include <list>
#include <map>
#include "util/api.hpp"
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/onestepofplannerresult.hpp>
#include <contextualplanner/types/problem.hpp>


namespace cp
{


/**
 * @brief Ask the planner to get the next action to do.
 * @param[in, out] pProblem Problem of the planner.
 * @param[in] pDomain Domain of the planner.
 * @param[in] pNow Current time.
 * @param[in, opt] pGlobalHistorical A historical to give more priority to an action less frequently used.<br/>
 * Note that the problem already have a historical. The problem historical has more priority than this historical.<br/>
 * This historical can be useful if you have different problem instances and you want to favorise the action less freqently used at a global scope.
 * @return One step of the planner containing the next action to do, his parameters and information about the goal that motivated that action.
 */
CONTEXTUALPLANNER_API
std::unique_ptr<OneStepOfPlannerResult> lookForAnActionToDo(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    const Historical* pGlobalHistorical = nullptr);


/**
 * @brief Notify that an action finished. This function will update the world (contained in the problem) accordingly.
 * @param[in, out] pProblem Problem of the planner.
 * @param[in] pDomain Domain of the planner.
 * @param[in] pOnStepOfPlannerResult Planner result step that motivated this action.
 * @param[in] pNow Current time.
 */
CONTEXTUALPLANNER_API
void notifyActionDone(Problem& pProblem,
                      const Domain& pDomain,
                      const OneStepOfPlannerResult& pOnStepOfPlannerResult,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);


/**
 * @brief Ask the planner to get all the actions to do until:<br/>
 *  * The goals are satisfied.
 *  * We cannot satisfy more goals.
 * @param[in, out] pProblem Problem of the planner.
 * @param[in] pDomain Domain of the planner.
 * @param[in] pNow Current time.
 * @param[in, out] pGlobalHistorical Historical more global (and with a smaller priority) than the one contained in the problem.<br/>
 * The historical is used to add diversity in the actions to do. In other words, it is to always do the same action if another action is pertinent too.
 * @return List of all the actions to do with their parameters with values.
 */
CONTEXTUALPLANNER_API
std::list<ActionInstance> lookForResolutionPlan(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical = nullptr);


/**
 * @brief Convert  a plan to a string.
 * @param[in] pPlan Plan to print.
 * @param[in] pSep Separator between each action of the plan to do sequentially.
 * @return The plan written in string.
 */
CONTEXTUALPLANNER_API
std::string planToStr(const std::list<cp::ActionInstance>& pPlan,
                      const std::string& pSep = ", ");

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
