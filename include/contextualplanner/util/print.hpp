#ifndef INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP
#define INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP

#include "api.hpp"
#include <map>
#include <list>
#include <contextualplanner/types/problem.hpp>
#include <contextualplanner/types/domain.hpp>


namespace cp
{

CONTEXTUALPLANNER_API
std::string printActionIdWithParameters(
    const std::string& pActionId,
    const std::map<std::string, std::string>& pParameters);



CONTEXTUALPLANNER_API
std::list<std::string> printResolutionPlan(Problem& pProblem,
                                           const Domain& pDomain,
                                           const std::unique_ptr<std::chrono::steady_clock::time_point>& pNow,
                                           Historical* pGlobalHistorical = nullptr);

}

#endif // INCLUDE_CONTEXTUALPLANNER_UTIL_PRINT_HPP
