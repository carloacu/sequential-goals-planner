#include <contextualplanner/types/entity.hpp>


namespace cp
{
namespace
{
const std::string _anyValue = "*";
}

Entity::Entity(std::string pValue,
               std::string pType)
 : value(pValue),
   type(pType)
{
}

Entity Entity::createAnyEntity()
{
  return Entity(_anyValue);
}


} // !cp
