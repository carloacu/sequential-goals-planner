#include <prioritizedgoalsplanner/types/type.hpp>
#include <stdexcept>

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


std::shared_ptr<Type> Type::getSmallerType(const std::shared_ptr<Type>& pType1,
                                           const std::shared_ptr<Type>& pType2)
{
  if (!pType1)
    return pType2;
  if (!pType2)
    return pType1;

  if (pType1->isA(*pType2))
    return pType1;

  if (pType2->isA(*pType1))
    return pType2;

  throw std::runtime_error("\"" + pType1->name + "\" and \"" + pType2->name + "\" types does not have a common parent");
}


} // !cp
