#include <contextualplanner/types/entity.hpp>
#include <vector>
#include <contextualplanner/types/parameter.hpp>
#include <contextualplanner/types/setoftypes.hpp>
#include <contextualplanner/util/util.hpp>


namespace cp
{
namespace
{
const std::string _anyValue = "*";
}

Entity::Entity(const std::string& pValue,
               const std::shared_ptr<Type>& pType)
 : value(pValue),
   type(pType)
{
}

Entity::Entity(Entity&& pOther) noexcept
  : value(std::move(pOther.value)),
    type(pOther.type) {
}


Entity& Entity::operator=(Entity&& pOther) noexcept {
    value = std::move(pOther.value);
    type = pOther.type;
    return *this;
}


bool Entity::operator<(const Entity& pOther) const {
  if (value != pOther.value)
    return value < pOther.value;
  if (type && !pOther.type)
    return true;
  if (!type && pOther.type)
    return false;
  if (type && pOther.type)
    return *type < *pOther.type;
  return false;
}


bool Entity::operator==(const Entity& pOther) const {
  return value == pOther.value && type == pOther.type;
}


const std::string& Entity::anyEntityValue()
{
  return _anyValue;
}


Entity Entity::createAnyEntity()
{
  return Entity(_anyValue);
}


Entity Entity::createNumberEntity(const std::string& pNumber,
                                  const SetOfTypes& pSetOfTypes)
{
  return Entity(pNumber, pSetOfTypes.numberType());
}


Entity Entity::fromStr(const std::string& pStr,
                       const SetOfTypes& pSetOfTypes)
{
  std::vector<std::string> nameWithType;
  cp::split(nameWithType, pStr, "-");

  if (nameWithType.empty())
    throw std::runtime_error("\"" + pStr + "\" is not a valid entity");

  if (nameWithType.size() > 1)
  {
    auto valueStr = nameWithType[0];
    trim(valueStr);
    auto typeStr = nameWithType[1];
    trim(typeStr);
    return Entity(valueStr, pSetOfTypes.nameToType(typeStr));
  }

  return Entity(nameWithType[0]);
}


std::string Entity::toStr() const
{
  if (!type)
    return value;
  return value + " - " + type->name;
}

bool Entity::isAnyValue() const
{
  return value == _anyValue;
}

bool Entity::isAParameterToFill() const
{
  return !value.empty() && (value[0] == '?' || isAnyValue());
}

bool Entity::match(const Parameter& pParameter) const
{
  if (value == pParameter.name)
  {
    if (type && pParameter.type && !type->isA(*pParameter.type))
      return false;
    return true;
  }
  return false;
}


bool Entity::isValidParameterAccordingToPossiblities(const std::vector<Parameter>& pParameters) const
{
  for (auto& currParam : pParameters)
  {
    if (value == currParam.name)
    {
      if (type && currParam.type && !currParam.type->isA(*type))
        return false;
      return true;
    }
  }
  return false;
}



} // !cp
