#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_SETOFENTITIES_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_SETOFENTITIES_HPP

#include "../util/api.hpp"
#include <list>
#include <map>
#include <set>
#include <string>
#include "entity.hpp"

namespace pgp
{
struct SetOfTypes;


struct PRIORITIZEDGOALSPLANNER_API SetOfEntities
{
  SetOfEntities();

  static SetOfEntities fromPddl(const std::string& pStr,
                               const SetOfTypes& pSetOfTypes);

  void add(const Entity& pEntity);

  void addAllFromPddl(const std::string& pStr,
                      const SetOfTypes& pSetOfTypes);

  const std::set<Entity>* typeNameToEntities(const std::string& pTypename) const;

  const Entity* valueToEntity(const std::string& pValue) const;

  std::string toStr(std::size_t pIdentation = 0) const;

  bool empty() const { return _valueToEntity.empty(); }

private:
  std::map<std::string, Entity> _valueToEntity;
  std::map<std::string, std::set<Entity>> _typeNameToEntities;
};

} // namespace pgp

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_SETOFENTITIES_HPP
