#include <contextualplanner/types/setofentities.hpp>
#include <vector>
#include <contextualplanner/util/util.hpp>
#include <contextualplanner/types/setoftypes.hpp>

namespace cp
{

SetOfEntities::SetOfEntities()
    : _entities(),
      _valueToEntity()
{
}


SetOfEntities SetOfEntities::fromStr(const std::string& pStr,
                                     const SetOfTypes& pSetOfTypes)
{
  SetOfEntities res;
  std::vector<std::string> lineSplitted;
  cp::split(lineSplitted, pStr, "\n");
  for (auto& currLine : lineSplitted)
  {
    std::vector<std::string> entitiesWithType;
    cp::split(entitiesWithType, currLine, "-");

    if (entitiesWithType.empty())
      continue;

    if (entitiesWithType.size() <= 1) {
      trim(currLine);
      if (currLine.empty())
        continue;
      throw std::runtime_error("Missing type for entities declaration: \"" + currLine + "\"");
    }

    std::string typeStr = entitiesWithType[1];
    trim(typeStr);
    auto type = pSetOfTypes.nameToType(typeStr);
    if (!type)
      throw std::runtime_error("\"" + typeStr + "\" is not a valid type name in line \"" + currLine + "\"");

    auto entitiesStr = entitiesWithType[0];
    std::vector<std::string> entitiesStrs;
    cp::split(entitiesStrs, entitiesStr, " ");
    for (auto& currEntity : entitiesStrs)
      if (!currEntity.empty())
        res.add(Entity(currEntity, type));
  }
  return res;
}


void SetOfEntities::add(const Entity& pEntity)
{
  _entities.push_back(pEntity);
  _valueToEntity[pEntity.value] = &_entities.back();
}

const Entity* SetOfEntities::valueToEntity(const std::string& pValue) const
{
  auto it = _valueToEntity.find(pValue);
  if (it != _valueToEntity.end())
    return it->second;
  return nullptr;
}

std::string SetOfEntities::toStr() const
{
  std::map<std::string, std::string> typeToValues;
  for (auto& currEntity : _entities)
  {
    std::string typeName;
    if (currEntity.type)
      typeName = currEntity.type->name;
    auto& valuesStr = typeToValues[typeName];
    if (valuesStr != "")
      valuesStr += " ";
    valuesStr += currEntity.value;
  }

  std::string res;
  bool firstIteration = true;
  for (auto& currTypeToValues : typeToValues)
  {
    if (firstIteration)
      firstIteration = false;
    else
      res += "\n";
    res += currTypeToValues.second;
    if (currTypeToValues.first != "")
      res += " - " + currTypeToValues.first;
  }
  return res;
}


} // !cp
