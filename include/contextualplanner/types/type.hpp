#ifndef INCLUDE_CONTEXTUALPLANNER_TYPES_TYPE_HPP
#define INCLUDE_CONTEXTUALPLANNER_TYPES_TYPE_HPP

#include "../util/api.hpp"
#include <string>
#include <list>

namespace cp
{

struct CONTEXTUALPLANNER_API Type
{
  Type(const std::string& pName,
       const Type* pParent = nullptr);

  void toStrs(std::list<std::string>& pStrs) const;

  const std::string name;
  const Type* parent;
  std::list<Type> subTypes;
};

} // namespace cp

#endif // INCLUDE_CONTEXTUALPLANNER_TYPES_TYPE_HPP
