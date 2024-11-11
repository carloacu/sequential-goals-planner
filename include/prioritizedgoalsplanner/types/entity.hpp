#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_ENTITY_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_ENTITY_HPP

#include <string>
#include <vector>
#include "../util/api.hpp"
#include "type.hpp"

namespace pgp
{
struct Ontology;
struct Parameter;
struct SetOfEntities;
struct SetOfTypes;


struct PRIORITIZEDGOALSPLANNER_API Entity
{
  Entity(const std::string& pValue,
         const std::shared_ptr<Type>& pType);

  Entity(const Entity& pOther) = default;
  Entity(Entity&& pOther) noexcept;
  Entity& operator=(const Entity& pOther) = default;
  Entity& operator=(Entity&& pOther) noexcept;

  bool operator<(const Entity& pOther) const;

  bool operator==(const Entity& pOther) const;
  bool operator!=(const Entity& pOther) const { return !operator==(pOther); }

  static const std::string& anyEntityValue();
  static Entity createAnyEntity();
  static Entity createNumberEntity(const std::string& pNumber);
  static Entity fromDeclaration(const std::string& pStr,
                                const SetOfTypes& pSetOfTypes);

  static Entity fromUsage(const std::string& pStr,
                          const Ontology& pOntology,
                          const SetOfEntities& pEntities,
                          const std::vector<Parameter>& pParameters);

  std::string toStr() const;
  bool isAnyValue() const;
  bool isAParameterToFill() const;
  Parameter toParameter() const;
  bool match(const Parameter& pParameter) const;
  bool isValidParameterAccordingToPossiblities(const std::vector<Parameter>& pParameter) const;

  std::string value;
  std::shared_ptr<Type> type;
};

} // !pgp


#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_ENTITY_HPP
