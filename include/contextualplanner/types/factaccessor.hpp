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
  FactAccessor(const Fact& pFact,
               bool pInAcontainer = false,
               bool pIgnoreFluent = false);

  static void conditonFactToListOfFactAccessors(std::list<FactAccessor>& pRes,
                                                const Fact& pFact);

  /// Comparator to find the fact from conditions related to a fact.
  bool operator<(const FactAccessor& pOther) const;


  const std::string& factSignature() const { return _factSignature; }
  bool hasParameters() const { return _hasParameters; }

private:
  /// Name of the fact.
  std::string _factSignature;

  bool _hasParameters;

  /// Value of each argument if the argument is a constant, an empty string otherwise.
  std::vector<std::string> _argumentConstantValues;

  /// Fluent value if the fluent is a constant, an empty string otherwise.
  std::string _fluentConstantValue;

  /**
   * True if all the arguments and the fluent of this object should be equal to the other object we compare.
   * False otherwise.
   */
  bool _inAContainer;

  bool _ignoreFluent;

  bool _compareBothFromConditionAccessors(const FactAccessor& pOther) const;

  bool _compareFromCondition(const FactAccessor& pOtherThatIsNotfromACondition) const;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACTACCESSOR_HPP
