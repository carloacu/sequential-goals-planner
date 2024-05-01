#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP

#include "../util/api.hpp"
#include <list>
#include <map>
#include <string>
#include "type.hpp"

namespace cp
{

struct CONTEXTUALPLANNER_API Ontology
{
  Ontology();

  void addType(const std::string& pTypeToAdd,
               const std::string& pParentType = "");

  std::list<std::string> typesToStrs() const;
  std::string typesToStr() const;

private:
  std::list<Type> _types;
  std::map<std::string, Type*> _nameToType;
};

} // namespace cp

#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_ONTOLOGY_HPP
