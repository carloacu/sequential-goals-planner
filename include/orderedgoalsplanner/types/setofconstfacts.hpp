#ifndef INCLUDE_ORDEREDGOALSPLANNER_SETOfCONSTFACTS_HPP
#define INCLUDE_ORDEREDGOALSPLANNER_SETOfCONSTFACTS_HPP

#include "../util/api.hpp"
#include <orderedgoalsplanner/types/setoffacts.hpp>


namespace ogp
{

struct ORDEREDGOALSPLANNER_API SetOfConstFacts
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

} // !ogp


#endif // INCLUDE_ORDEREDGOALSPLANNER_SETOfCONSTFACTS_HPP
