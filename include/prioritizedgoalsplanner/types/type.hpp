#ifndef INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_TYPE_HPP
#define INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_TYPE_HPP

#include "../util/api.hpp"
#include <string>
#include <list>
#include <memory>

namespace pgp
{

struct PRIORITIZEDGOALSPLANNER_API Type
{
  Type(const std::string& pName,
       const std::shared_ptr<Type>& pParent = {});

  Type(const Type& pOther) = default;
  Type(Type&& pOther) noexcept = delete;
  Type& operator=(const Type& pOther) = default;
  Type& operator=(Type&& pOther) noexcept = delete;

  void toStrs(std::list<std::string>& pStrs) const;

  bool isA(const Type& pOtherType) const;

  bool operator<(const Type& pOther) const;

  static std::shared_ptr<Type> getSmallerType(const std::shared_ptr<Type>& pType1,
                                              const std::shared_ptr<Type>& pType2);

  const std::string name;
  const std::shared_ptr<Type> parent;
  std::list<std::shared_ptr<Type>> subTypes;
};

} // namespace pgp

#endif // INCLUDE_PRIORITIZEDGOALSPLANNER_TYPES_TYPE_HPP
