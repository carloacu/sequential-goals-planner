#ifndef INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP
#define INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP

#include "../util/api.hpp"
#include <contextualplanner/types/setoffacts.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API SetOfConstFacts
{
  void add(const Fact& pFact) { _setOfFacts.add(pFact, false); }

  const SetOfFact& setOfFacts() const { return _setOfFacts; }

private:
  SetOfFact _setOfFacts{};
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP
