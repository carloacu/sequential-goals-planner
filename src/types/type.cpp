#include <contextualplanner/types/type.hpp>


namespace cp
{

Type::Type(const std::string& pName,
           const std::shared_ptr<Type>& pParent)
    : name(pName),
      parent(pParent),
      subTypes()
{
}


void Type::toStrs(std::list<std::string>& pStrs) const
{
  // Declare type
  if (subTypes.empty())
  {
    if (parent == nullptr)
      pStrs.emplace_back(name);
  }
  else
  {
    std::string subTypesStr;
    for (auto& currType : subTypes)
    {
      if (!subTypesStr.empty())
        subTypesStr += " ";
      subTypesStr += currType->name;
    }
    pStrs.emplace_back(subTypesStr + " - " + name);

    for (auto& currType : subTypes)
      currType->toStrs(pStrs);
  }
}


bool Type::isA(const Type& pOtherType) const
{
  if (name == pOtherType.name)
    return true;
  if (parent)
    return parent->isA(pOtherType);
  return false;
}


bool Type::operator<(const Type& pOther) const
{
  if (name != pOther.name)
    return name < pOther.name;
  return false;
}


} // !cp
