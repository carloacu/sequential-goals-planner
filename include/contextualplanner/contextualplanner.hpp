#ifndef INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
#define INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP

#include <map>
#include "util/api.hpp"
#include <contextualplanner/util/alias.hpp>
#include <contextualplanner/types/domain.hpp>
#include <contextualplanner/types/problem.hpp>


namespace cp
{

/**
 * @brief Ask the planner to get the next action to do.
 * @param[out] pParameters Parameters for the next action to do.
 * @param[in, out] pProblem Problem of the planner.
 * @param[in] pDomain Domain of the planner.
 * @param[in] pNow Current time.
 * @param[out, opt] pGoalOfTheAction Goal that motivated to do the returned action.
 * @param[out, opt] pGoalPriority The priority of the goal that motivated to do the returned action.
 * @param[in, opt] pGlobalHistorical A historical to give more priority to an action less frequently used.<br/>
 * Note that the problem already have a historical. The problem historical has more priority than this historical.<br/>
 * This historical can be useful if you have different problem instances and you want to favorise the action less freqently used at a global scope.
 * @return The next action to do.
 */
CONTEXTUALPLANNER_API
ActionId lookForAnActionToDo(std::map<std::string, std::string>& pParameters,
                             Problem& pProblem,
                             const Domain& pDomain,
                             const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                             const Goal** pGoalOfTheAction = nullptr,
                             int* pGoalPriority = nullptr,
                             const Historical* pGlobalHistorical = nullptr);

/**
 * @brief Notify that an action finished. This function will update the world (contained in the problem) accordingly.
 * @param[in, out] pProblem Problem of the planner.
 * @param[in] pDomain Domain of the planner.
 * @param[in] pActionId Identifier of the finished action.
 * @param[out] pParameters Parameters of the finished action.
 * @param[in] pNow Current time.
 */
CONTEXTUALPLANNER_API
void notifyActionDone(Problem& pProblem,
                      const Domain& pDomain,
                      const std::string& pActionId,
                      const std::map<std::string, std::string>& pParameters,
                      const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);



/// Struct gathering the identifier of an action with some parameter values.
struct CONTEXTUALPLANNER_API ActionWithHisParameters
{
  /// Construct ActionWithHisParameters.
  ActionWithHisParameters(const std::string& pActionId,
                         const std::map<std::string, std::string>& pParameters);

  /// Identifer of the action.
  std::string actionId;
  /// Action parameter values.
  std::map<std::string, std::string> parameters;
};


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
std::list<ActionWithHisParameters> lookForResolutionPlan(
    Problem& pProblem,
    const Domain& pDomain,
    const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
    Historical* pGlobalHistorical = nullptr);

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_CONTEXTUALPLANNER_HPP
