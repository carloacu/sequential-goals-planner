#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP

#include "api.hpp"
#include <map>
#include <list>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/types/domain.hpp>


namespace cp
{

/**
 * @brief Print the goals separated by ", ".
 * @param pGoals Goals to print.
 * @return The string of printed goals.
 */
CONTEXTUALPLANNER_API
std::string printGoals(const std::map<int, std::vector<Goal>>& pGoals);


/**
 * @brief Print a table of goals with 3 columns:
 *  * Name (string value of the goal)
 *  * Stack time (how log the goal is stacked. (stacked = not on the top of the stack))
 *  * Max stack time (max time that the goal can be stacked)
 * @param pGoals Goals to print.
 * @param pNow Current time.
 * @return The string of the table of goals.
 */
CONTEXTUALPLANNER_API
std::string printGoalsTable(std::size_t pGoalNameMaxSize,
                            const std::map<int, std::vector<Goal>>& pGoals,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP
