#ifndef INCLUDE_CONTEXTUALPLANNER_FACT_HPP
#define INCLUDE_CONTEXTUALPLANNER_FACT_HPP

#include <string>
#include <vector>
#include <map>
#include "../util/api.hpp"


namespace cp
{

/// Axiomatic knowledge that can be contained in the world.
struct CONTEXTUALPLANNER_API Fact
{
  /**
   * @brief Construct a fact.
   * @param pName Name of the fact.
   */
  Fact(const std::string& pName = "");

  /// Specify an order beween facts. It alows to use this type as key of map containers.
  bool operator<(const Fact& pOther) const;

  /// Check equality with another Fact.
  bool operator==(const Fact& pOther) const;
  /// Check not equality with another ExpressionElement.
  bool operator!=(const Fact& pOther) const { return !operator==(pOther); }

  /**
   * @brief Is equal to another Fact or if any of the 2 Fact has an "any value" that can match.
   * @param pOther Other fact to compare.
   * @return True if the 2 facts match, false otherwise.
   */
  bool areEqualExceptAnyValues(const Fact& pOther) const;

  /**
   * @brief Extract a value of another fact from a parameter value of this fact.
   * @param pParameterValue Parameter value of this fact.
   * @param pOther Other Fact.
   * @return Value of the other fact cooresponding to the parameter value of this fact.
   */
  std::string tryToExtractParameterValueFromExemple(
      const std::string& pParameterValue,
      const Fact& pOther) const;

  /**
   * @brief Fill the parameters.
   * @param pParameters Parameters to fill.
   */
  void fillParameters(
      const std::map<std::string, std::string>& pParameters);

  /// Serialize this fact to a string.
  std::string toStr() const;

  /// Deserialize a string to a fact.
  static Fact fromStr(const std::string& pStr);

  /**
   * @brief Deserialize a part of a string to this fact.
   * @param pStr String containing the part to deserialize.
   * @param pBeginPos Begin index of the string for the deserialization.
   * @param pSeparator Character to specify the end of the fact.
   * @return End index of the deserialization.
   */
  std::size_t fillFactFromStr(
      const std::string& pStr,
      std::size_t pBeginPos,
      char pSeparator);

  /**
   * @brief Set "any value" to all of the specified parameters.
   * @param pParameters Parameters to set "any value".
   * @return True if at least one "any value" has been set, false otherwise.
   */
  bool replaceParametersByAny(const std::vector<std::string>& pParameters);

  /// Name of the fact.
  std::string name;
  /// Parameters of the fact.
  std::vector<Fact> parameters;
  /// Value of the fact.
  std::string value;

  /// Constant defining the "any value" special value.
  const static std::string anyValue;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_FACT_HPP
