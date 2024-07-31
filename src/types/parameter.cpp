#include <contextualplanner/types/parameter.hpp>
#include <vector>
#include <contextualplanner/types/setoftypes.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{


Parameter::Parameter(const std::string& pName,
                     const std::shared_ptr<Type>& pType)
  : name(pName),
    type(pType)
{
}

Parameter::Parameter(Parameter&& pOther) noexcept
  : name(std::move(pOther.name)),
    type(pOther.type) {
}


Parameter& Parameter::operator=(Parameter&& pOther) noexcept {
  name = std::move(pOther.name);
  type = pOther.type;
  return *this;
}


bool Parameter::operator<(const Parameter& pOther) const {
   // Because 2 parameters cannot have the same name
   return name < pOther.name;
}



bool Parameter::operator==(const Parameter& pOther) const {
  return name == pOther.name && type == pOther.type;
}


Parameter Parameter::fromStr(const std::string& pStr,
                             const SetOfTypes& pSetOfTypes)
{
  std::vector<std::string> nameWithType;
  cp::split(nameWithType, pStr, "-");

  if (nameWithType.empty())
    throw std::runtime_error("\"" + pStr + "\" is not a valid entity");

  if (nameWithType.size() > 1)
  {
    auto nameStr = nameWithType[0];
    trim(nameStr);
    auto typeStr = nameWithType[1];
    trim(typeStr);
    return Parameter(nameStr, pSetOfTypes.nameToType(typeStr));
  }

  return Parameter(nameWithType[0]);
}


std::string Parameter::toStr() const
{
  if (!type)
    return name;
  return name + " - " + type->name;
}


} // !cp
