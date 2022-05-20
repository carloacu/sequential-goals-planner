#ifndef INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP
#define INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP

#include <set>
#include <list>
#include <set>
#include "api.hpp"
#include <contextualplanner/fact.hpp>
#include <contextualplanner/expression.hpp>


namespace cp
{


struct CONTEXTUALPLANNER_API SetOfFacts
{
  SetOfFacts()
    : facts(),
      notFacts(),
      exps()
  {
  }
  SetOfFacts(const std::initializer_list<Fact>& pFacts,
             const std::initializer_list<Fact>& pNotFacts = {})
    : facts(pFacts),
      notFacts(pNotFacts),
      exps()
  {
  }
  bool empty() const { return facts.empty() && notFacts.empty() && exps.empty(); }
  bool operator==(const SetOfFacts& pOther) const
  { return facts == pOther.facts && notFacts == pOther.notFacts && exps == pOther.exps; }
  void add(const SetOfFacts& pOther)
  {
    facts.insert(pOther.facts.begin(), pOther.facts.end());
    notFacts.insert(pOther.notFacts.begin(), pOther.notFacts.end());
    exps.insert(exps.begin(), pOther.exps.begin(), pOther.exps.end());
  }
  bool containsFact(const Fact& pFact) const;
  void rename(const Fact& pOldFact,
              const Fact& pNewFact);
  std::list<std::pair<std::string, std::string>> toFactsStrs() const;
  std::list<std::string> toStrs() const;
  std::string toStr(const std::string& pSeparator) const;
  static SetOfFacts fromStr(const std::string& pStr,
                            char pSeparator);

  std::set<Fact> facts;
  std::set<Fact> notFacts;
  std::list<Expression> exps;
};


CONTEXTUALPLANNER_API
std::vector<cp::Fact> factsFromString(const std::string& pStr,
                                      char pSeparator);


} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_SETOFFACTS_HPP
