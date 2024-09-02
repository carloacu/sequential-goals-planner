#ifndef INCLUDE_CONTEXTUALPLANNER_FACTACCESSOR_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACTACCESSOR_HPP

#include <list>
#include <string>
#include <vector>
#include "../util/api.hpp"


namespace cp
{
struct Fact;

/// Class to retieve a fact from the conditions.
struct CONTEXTUALPLANNER_API FactAccessor
{
  /// Constructor.
  FactAccessor(const Fact& pFact);

  static void conditonFactToListOfFactAccessors(std::list<FactAccessor>& pRes,
                                                const Fact& pFact);


  const std::string& factSignature() const { return _factSignature; }

private:
  /// Name of the fact.
  std::string _factSignature;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTACCESSOR_HPP
