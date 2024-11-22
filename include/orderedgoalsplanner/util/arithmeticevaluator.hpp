#ifndef INCLUDE_ORDEREDGOALSPLANNER_ARITHMETICEVALUATOR_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_ARITHMETICEVALUATOR_HPP

#include <string>
#include "api.hpp"

namespace ogp
{

ORDEREDGOALSPLANNER_API
int evalute(const std::string& pText,
            std::size_t pBeginPos = 0);

ORDEREDGOALSPLANNER_API
std::string evaluteToStr(const std::string& pText,
                         std::size_t pBeginPos = 0);

}

#endif // INCLUDE_ORDEREDGOALSPLANNER_ARITHMETICEVALUATOR_HPP
