#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_UTIL_SERIALIZER_SERIALIZEINPDDL_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_UTIL_SERIALIZER_SERIALIZEINPDDL_HPP

#include <string>

namespace pgp
{
struct Condition;
struct Domain;
struct Problem;


std::string domainToPddl(const Domain& pDomain);

std::string problemToPddl(const Problem& pProblem,
                          const Domain& pDomain);


std::string conditionToPddl(const Condition& pCondition,
                            std::size_t pIdentation);

} // End of namespace pgp



#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_UTIL_SERIALIZER_SERIALIZEINPDDL_HPP
