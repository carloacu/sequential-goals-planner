#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_SETOfCONSTFACTS_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_SETOfCONSTFACTS_HPP

#include "../util/api.hpp"
#include <prioritizedgoalsplanner/types/setoffacts.hpp>


namespace pgp
{

struct PRIORITIZEDGOALSPLANNER_API SetOfConstFacts
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

} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_SETOfCONSTFACTS_HPP
