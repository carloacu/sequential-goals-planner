#ifndef INCLUDE_CONTEXTUALPLANNER_PREDICATE_HPP
#define INCLUDE_CONTEXTUALPLANNER_PREDICATE_HPP

#include <optional>
#include <string>
#include <vector>
#include "../util/api.hpp"


namespace cp
{

struct CONTEXTUALPLANNER_API Predicate
{
  Predicate(const std::string& pStr);

  std::string toStr() const;

  /// Name of the predicate.
  std::string name;
  /// Argument types of the predicate.
  std::vector<std::string> parameters;
  /// Fluent type of the predicate.
  std::optional<std::string> fluent;
};

} // !cp


#endif // INCLUDE_CONTEXTUALPLANNER_PREDICATE_HPP
