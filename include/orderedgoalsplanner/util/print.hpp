#ifndef INCLUDE_ORDEREDGOALSPLANNER_UTIL_PRINT_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_UTIL_PRINT_HPP

#include "api.hpp"
#include <map>
#include <list>
#include <orderedgoalsplanner/types/problem.hpp>
#include <orderedgoalsplanner/types/domain.hpp>


namespace ogp
{

/**
 * @brief Print the goals separated by ", ".
 * @param pGoals Goals to print.
 * @return The string of printed goals.
 */
ORDEREDGOALSPLANNER_API
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
ORDEREDGOALSPLANNER_API
std::string printGoalsTable(std::size_t pGoalNameMaxSize,
                            const std::map<int, std::vector<Goal>>& pGoals,
                            const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow);

}

#endif // INCLUDE_ORDEREDGOALSPLANNER_UTIL_PRINT_HPP
