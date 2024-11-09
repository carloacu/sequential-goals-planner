#ifndef INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP
#define INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP

#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/setoffacts.hpp>


namespace cp
{

struct CONTEXTUALPLANNER_API SetOfConstFacts
{
  static SetOfConstFacts fromPddl(const std::string& pStr,
                                  std::size_t& pPos,
                                  const Ontology& pOntology,
                                  const SetOfEntities& pEntities) {
    SetOfConstFacts res;
    res._setOfFacts = SetOfFacts::fromPddl(pStr, pPos, pOntology, pEntities, false);
    return res;
  }

  void add(const Fact& pFact) { _setOfFacts.add(pFact, false); }

  const SetOfFacts& setOfFacts() const { return _setOfFacts; }

private:
  SetOfFacts _setOfFacts{};
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_SETOfCONSTFACTS_HPP
