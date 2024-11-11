#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_ARITHMETICEVALUATOR_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_ARITHMETICEVALUATOR_HPP

#include <string>
#include "api.hpp"

namespace cp
{

PRIORITIZEDGOALSPLANNER_API
int evalute(const std::string& pText,
            std::size_t pBeginPos = 0);

PRIORITIZEDGOALSPLANNER_API
std::string evaluteToStr(const std::string& pText,
                         std::size_t pBeginPos = 0);

}

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_ARITHMETICEVALUATOR_HPP
